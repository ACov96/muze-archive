// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"
#include "context.h"
#include "morph_graph.h"

/* MACROS */
#define WORD 8
#define MODULE_MIN_SIZE 16
#define NO_OPERANDS ""
#define CREATE_BUFFER char *buf = ""
#define RETURN_BUFFER return buf
#define PRINT_BUFFER printf("%s\n", buf)

#define INT_LITERAL(I) (concat("$", itoa(I)))

#define ADD_BLOCK(B) buf = concat(buf, B);      \
  buf = concat(buf, "\n");                                           

#define ADD_LABEL(L) buf = concat(buf, L);      \
  buf = concat(buf, ":\n")

#define ADD_INSTR(I, O) buf = concat(buf, "\t");        \
  buf = concat(buf, I);                                 \
  buf = concat(buf, "\t");                              \
  buf = concat(buf, O);                                 \
  buf = concat(buf, "\n")

#define GEN_ERROR(S) fprintf(stderr, "Codegen Error: %s\n", S); \
  exit(1)

#define GEN_WARN(S) printf("Codegen Warning: %s\n", S)

/* TYPES */
typedef struct label_st        *label_t;
typedef struct string_label_st *string_label_t;
typedef char                   *reg_t;

struct label_st {
  char         *label;
  unsigned int  count;
};

struct string_label_st {
  char    *str;
  char    *label;
};

/* GLOBALS */
type_node_t *graph;
ll_t labels = NULL;
ll_t strings = NULL;
unsigned int curr_label = 0;
reg_t arg_registers[6] = {
                        "%rdi",
                        "%rsi",
                        "%rdx",
                        "%rcx",
                        "%r8",
                        "%r9"
};
char *entrypoint = NULL;

/* PROTOTYPES */
// Helper functions
unsigned int count_consts_and_vars(decl_t decl);
int get_id_offset(char *id, decl_t decl);
char* register_or_get_string_label(char *str);
void populate_decl_into_ctx(context_t ctx, decl_t decl);
void set_entrypoint(root_t root);

// Generator functions
char* gen_label(char *label);
char* gen_data_segment();
char* gen_mod(context_t ctx, mod_t mod);
char* gen_fun(context_t ctx, fun_decl_t fun);
char* gen_stmt(context_t ctx, stmt_t stmt);
char* gen_expr_stmt(context_t ctx, expr_stmt_t expr);
char* gen_expr(context_t ctx, expr_t expr, reg_t out);
char* gen_call_expr(context_t ctx, call_t call, reg_t out);
char* gen_literal_expr(context_t ctx, literal_t literal, reg_t out);
char* gen_id_expr(context_t ctx, char *id, reg_t out);
char* gen_assign_stmt(context_t ctx, assign_stmt_t assign);
char* gen_lval_expr(context_t ctx, expr_t lval, reg_t out);
char* gen_cond_stmt(context_t ctx, cond_stmt_t cond);
char* gen_loop_stmt(context_t ctx, loop_stmt_t loop);
char* gen_break_stmt(context_t ctx, break_stmt_t brk);
char* gen_binary_expr(context_t ctx, binary_t binary, reg_t out);
char* gen_ternary_expr(context_t ctx, ternary_t ternary, reg_t out);
char* gen_unary_expr(context_t ctx, unary_t unary, reg_t out);

/* HELPERS */
unsigned int count_consts_and_vars(decl_t decl) {
  unsigned int total = 0;
  for (const_decl_t c = decl->constants; c; c = c->next)
    total++;
  for (var_decl_t var = decl->vars; var; var = var->next)
    for (id_list_t id = var->names; id; id = id->next)
      total++;
  return total * WORD;
}

void populate_decl_into_ctx(context_t ctx, decl_t decl) {
  // TODO: This only considers name types, not records or morphs
  for (const_decl_t c = decl->constants; c; c = c->next)
    ctx_add_constant(ctx, c->name, c->ty->u.name_ty);
  for (var_decl_t v = decl->vars; v; v = v->next)
    for (id_list_t id = v->names; id; id = id->next)
      ctx_add_variable(ctx, id->name, v->type->u.name_ty);
}

int get_id_offset(char *id, decl_t decl) {
  int offset = 0;
  for (const_decl_t c = decl->constants; c; c = c->next) {
    if (strcmp(id, c->name) == 0) return offset;
    offset++;
  }
  for (var_decl_t v = decl->vars; v; v = v->next) {
    for (id_list_t i = v->names; i; i = i->next) {
      if (strcmp(i->name, id) == 0) return offset;
      offset++;
    }
  }
  // TODO: Change this to an actual error message and quit
  return -1;
}

char* register_or_get_string_label(char *str) {
  if (!strings) {
    strings = ll_new(); 
    string_label_t str_label = malloc(sizeof(struct string_label_st));
    strings->val = str_label;
    str_label->str = str;
    str_label->label = gen_label("STR");
    return str_label->label;
  }
  for (ll_t l = strings; l; l = l->next) {
    string_label_t sl = l->val;
    if (strcmp(sl->str, str) == 0)
      return sl->label;
  }
  string_label_t str_label = malloc(sizeof(struct string_label_st));
  str_label->str = str;
  str_label->label = gen_label("STR");
  ll_append(strings, str_label);
  return str_label->label;
}

void set_entrypoint(root_t root) {
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    for (fun_decl_t f = mod->decl->funs; f; f = f->next) {
      if (strcmp(f->name, "main") == 0) {
        entrypoint = concat(mod->name, concat("_", f->name));
      }
    }
  }
}

/* GENERATORS */
char* gen_label(char *label) {
  char *new_label = NULL;
  if (!labels) {
    labels = ll_new(); 
    label_t l = malloc(sizeof(struct label_st));
    l->label = label;
    l->count = 0;
    new_label = concat(label, itoa(l->count));
    ll_append(labels, l);
  } else {
    for (ll_t l = labels; l; l = l->next) {
      label_t curr = l->val;
      if (strcmp(curr->label, label) == 0) {
        curr->count++;
        new_label = concat(label, itoa(curr->count));
      }
    }  
    if (!new_label) {
      // We didn't find the label we were looking for in the linked list, create it
      label_t l = malloc(sizeof(struct label_st));
      l->label = label;
      l->count = 0;
      new_label = concat(label, itoa(l->count));
      ll_append(labels, l);
    }
  }
  return new_label;
}

char* gen_mod(context_t ctx, mod_t mod) {
  CREATE_BUFFER;
  if (strlen(ctx_get_scope_name(ctx)) == 0) {
    ctx_set_scope_name(ctx, mod->name);
  } else {
    ctx_set_scope_name(ctx, concat(ctx_get_scope_name(ctx), concat("_", mod->name)));
  }
  for (fun_decl_t f = mod->decl->funs; f; f = f->next) {
    ctx_add_function(ctx, f);
  }
  populate_decl_into_ctx(ctx, mod->decl);

  // Allocate module block
  ADD_LABEL(concat("__module__", concat(ctx_get_scope_name(ctx), "_init")));

  // Populate module block with constants and variables
  int mod_size = MODULE_MIN_SIZE;
  for (const_decl_t c = mod->decl->constants; c; c = c->next) {
    mod_size += WORD;
  }
  for (var_decl_t v = mod->decl->vars; v; v = v->next) {
    // TODO: This is incorrect and should loop over the id list
    mod_size += WORD;
  }
  ADD_INSTR("push", "%r15");
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("movq", concat(INT_LITERAL(mod_size), ", %rdi"));
  ADD_INSTR("call", "malloc");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("movq", "%rax, %r15");

  int offset = 1;
  for (const_decl_t c = mod->decl->constants; c; c = c->next) {
    assign_t assign = c->assign;
    ADD_BLOCK(gen_expr(ctx, assign->expr, "%rax"));
    ADD_INSTR("movq", concat("%rax, ", concat(itoa(offset * WORD), "(%r15)")));
    offset++;
  }
  for (var_decl_t v = mod->decl->vars; v; v = v->next) {
    assign_t assign = v->assign;
    ADD_BLOCK(gen_expr(ctx, assign->expr, "%rax"));
    ADD_INSTR("movq", concat("%rax, ", concat(itoa(offset * WORD), "(%r15)")));
    offset++;
  }
  ADD_INSTR("movq", "%r15, %rax");
  ADD_INSTR("pop", "%r15");
  ADD_INSTR("ret", NO_OPERANDS);

  // Define sub modules

  // Define type morph functions

  // Define functions
  for (fun_decl_t f = mod->decl->funs; f; f = f->next) {
    context_t func_ctx = ctx_new();
    ctx_set_scope_name(func_ctx, ctx_get_scope_name(ctx));
    ctx_set_parent(func_ctx, ctx);
    ctx_set_func(func_ctx);
    ADD_BLOCK(gen_fun(func_ctx, f));
  }
  RETURN_BUFFER;
}

char* gen_fun(context_t ctx, fun_decl_t fun) {
  CREATE_BUFFER;
  ctx_set_scope_name(ctx, concat(ctx_get_scope_name(ctx), concat("_", fun->name)));
  populate_decl_into_ctx(ctx, fun->decl);
  for (arg_t arg = fun->args; arg; arg = arg->next) {
    // TODO: Add record and morph types
    ctx_add_argument(ctx, arg->name, arg->type->u.name_ty);
  }
  for (fun_decl_t f = fun->decl->funs; f; f = f->next) {
    ctx_add_function(ctx, f);
  }

  // TODO: Generate child modules
  
  // Generate child functions
  for (fun_decl_t f = fun->decl->funs; f; f = f->next) {
    context_t new_ctx = ctx_new();
    ctx_set_parent(new_ctx, ctx);
    ctx_set_scope_name(new_ctx, ctx_get_scope_name(ctx));
    ADD_BLOCK(gen_fun(new_ctx, f));
  }
  
  // Calculate activation record space
  unsigned int preamble_space = count_consts_and_vars(fun->decl);
  for (arg_t arg = fun->args; arg; arg = arg->next) {
    preamble_space += WORD;
  }
  preamble_space += WORD; // Increment for static link
  char *space_str = concat("$", itoa(preamble_space));

  // Initialize activation record
  ADD_LABEL(ctx_get_scope_name(ctx));
  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", concat(space_str, ", %rsp"));

  // Setup static link
  ADD_INSTR("movq", "%r15, -8(%rbp)");
  
  // Push arguments to stack
  unsigned int idx = 1;
  for (arg_t arg = fun->args; arg; arg = arg->next) {
    // TODO: Make sure it handles 6+ arguments correctly. This only does registers
    char *save_inst = malloc(64);
    sprintf(save_inst, "%s, -%d(%%rbp)", arg_registers[idx-1], (idx + 1) * WORD);
    ADD_INSTR("movq", save_inst);
    idx++;
  }

  // Push constants and variables to stack
  for (const_decl_t c = fun->decl->constants; c; c = c-> next) {
    // TODO: Handle the different kinds of assignment
    ADD_BLOCK(gen_expr(ctx, c->assign->expr, "%rax"));
    char *save_inst = malloc(64);
    sprintf(save_inst, "%%rax, -%d(%%rbp)", (idx + 1) * WORD);
    ADD_INSTR("movq", save_inst);
    idx++;
  }

  for (var_decl_t v = fun->decl->vars; v; v = v->next) {
    // TODO: Handle the different kinds of assignment
    expr_t expr = v->assign->expr;
    for (id_list_t id = v->names; id; id = id->next) {
      char *x = gen_expr(ctx, expr, "%rax");
      ADD_BLOCK(x);
      char *save_inst = concat("%rax, -", concat(itoa((idx+1)*WORD), "(%rbp)"));
      ADD_INSTR("movq", save_inst);
      idx++;
    }
  }

  // Start executing statements
  for (stmt_t s = fun->stmts; s; s = s->next) {
    ADD_BLOCK(gen_stmt(ctx, s));
  }

  // Cleanup activation record
  ADD_INSTR("movq", "$0, %rax");
  ADD_INSTR("leave", NO_OPERANDS);
  ADD_INSTR("ret", NO_OPERANDS);
  RETURN_BUFFER;
}

char* gen_stmt(context_t ctx, stmt_t stmt) {
  CREATE_BUFFER;
  switch(stmt->kind) {
  case EXPR_STMT:
    ADD_BLOCK(gen_expr_stmt(ctx, stmt->u.expr_stmt));
    break;
  case COND_STMT:
    ADD_BLOCK(gen_cond_stmt(ctx, stmt->u.cond_stmt));
    break;
  case FOR_STMT:
    // TODO
    GEN_ERROR("For statement not implemented");
    break;
  case LOOP_STMT:
    ADD_BLOCK(gen_loop_stmt(ctx, stmt->u.loop_stmt));
    break;
  case CASE_STMT:
    // TODO
    GEN_ERROR("Case statement not implemented");
    break;
  case ASSIGN_STMT:
    ADD_BLOCK(gen_assign_stmt(ctx, stmt->u.assign_stmt));
    break;
  case BREAK_STMT:
    ADD_BLOCK(gen_break_stmt(ctx, stmt->u.break_stmt));
    break;
  default:
    GEN_ERROR("Unrecognized statement");
    break;
  }
  RETURN_BUFFER;
}

char* gen_expr_stmt(context_t ctx, expr_stmt_t expr) {
  return gen_expr(ctx, expr->expr, "%rax");
}

char* gen_expr(context_t ctx, expr_t expr, reg_t out) {
  CREATE_BUFFER;
  switch(expr->kind) {
  case ID_EX:
    ADD_BLOCK(gen_id_expr(ctx, expr->u.id_ex, out));
    break;
  case LITERAL_EX:
    ADD_BLOCK(gen_literal_expr(ctx, expr->u.literal_ex, out));
    break;
  case UNARY_EX:
    ADD_BLOCK(gen_unary_expr(ctx, expr->u.unary_ex, out));
    break;
  case BINARY_EX:
    ADD_BLOCK(gen_binary_expr(ctx, expr->u.binary_ex, out));
    break;
  case TERNARY_EX:
    ADD_BLOCK(gen_ternary_expr(ctx, expr->u.ternary_ex, out));
    break;
  case CALL_EX:
    ADD_BLOCK(gen_call_expr(ctx, expr->u.call_ex, out));
    break;
  case RANGE_EX:
    // TODO
    GEN_ERROR("Range expression not implemented");
    break;
  default:
    GEN_ERROR("Unrecognized expression");
    break;
  }
  RETURN_BUFFER;
}

char* gen_call_expr(context_t ctx, call_t call, reg_t out) {
  int arg_idx = 0;
  arg_t args = ctx_get_function_args(ctx, call->id);
  CREATE_BUFFER;

  // Put arguments in registers
  for (expr_list_t curr_arg = call->args; curr_arg; curr_arg = curr_arg->next) {
    ADD_INSTR("push", arg_registers[arg_idx]);
    char *temp = gen_expr(ctx, curr_arg->expr, arg_registers[arg_idx]);
    ADD_BLOCK(temp);
    
    // TODO: This logic will actually shrink once type checking is done because
    // all we need to do is get the type off of the expression
    if (args && curr_arg->expr->kind == ID_EX) {
      char *arg_type = ctx_get_id_type(ctx, curr_arg->expr->u.id_ex);
      char *target_type = args->type->u.name_ty;


      ADD_INSTR("push", "%rdi");
      ADD_INSTR("movq", concat(arg_registers[arg_idx], ", %rdi"));
      ADD_INSTR("call", concat("__morph__", concat(arg_type, concat("_", target_type))));
      ADD_INSTR("pop", "%rdi");
      ADD_INSTR("movq", concat("%rax, ", arg_registers[arg_idx]));
    }

    arg_idx++;
    if (args != NULL)
      args = args->next;
  }

  // Pass static link
  ADD_INSTR("push", "%r15");

  // TODO: Only change the static link if function is inner function 

  // Make call to function
  func_link_t fl = ctx_get_function(ctx, call->id);
  if (fl == NULL) {
    // TODO: This basically means if we can't find the function, just call the name
    ADD_INSTR("call", call->id);
  } else {
    ADD_INSTR("leaq", "-8(%rbp), %r15");
    for (int i = 0; i < fl->levels; i++) {
      ADD_INSTR("movq", "(%r15), %r15");
    }  
    ADD_INSTR("call", fl->id);
  }

  // Restore argument registers
  ADD_INSTR("pop", "%r15");
  for (int i = arg_idx - 1; i >= 0; i--) {
    ADD_INSTR("pop", arg_registers[i]);
  }

  if (strcmp("%rax", out) != 0) {
    ADD_INSTR("movq", concat("%rax, ", out));
  }
  RETURN_BUFFER;
}

char* gen_literal_expr(context_t ctx, literal_t literal, reg_t out) {
  char *str_label = NULL;
  char *int_literal = NULL;
  char *real_literal = NULL;
  char *bool_literal = NULL;
  CREATE_BUFFER;
  switch(literal->kind) {
  case STRING_LIT:
    str_label = concat("$", register_or_get_string_label(literal->u.string_lit));
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("movq", concat(str_label, ", %rdi"));
    ADD_INSTR("call", "alloc_str");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("movq", concat("%rax, ", out));
    break;
  case INTEGER_LIT:
    int_literal = concat("$", literal->u.integer_lit);
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("movq", concat(int_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_int");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("movq", concat("%rax, ", out));
    break;
  case REAL_LIT:
    real_literal = malloc(64);
    sprintf(real_literal, "$%lu", f_to_int(literal->u.real_lit));
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("movq", concat(real_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_real");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("movq", concat("%rax, ", out));
    break;
  case BOOLEAN_LIT:
    bool_literal = concat("$", itoa(literal->u.bool_lit == TRUE_BOOL));
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("movq", concat(bool_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_bool");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("movq", concat("%rax, ", out));
    break;
  default:
    GEN_ERROR("Unknown literal");
  }
  RETURN_BUFFER;
}

char* gen_data_segment() {
  // if (strings == NULL) return "";
  CREATE_BUFFER;
  ADD_BLOCK("\t.data\n");
  for (ll_t l = strings; l; l = l->next) {
    string_label_t str_label = l->val;
    ADD_LABEL(str_label->label);
    ADD_INSTR(".asciz", concat("\"", concat(str_label->str, "\"")));
  }
  RETURN_BUFFER;
}

char* gen_id_expr(context_t ctx, char *id, reg_t out) {
  CREATE_BUFFER;
  char *read_inst = malloc(64);
  static_link_t link = NULL;
  static_link_t sl = ctx_get_id_offset(ctx, id);
  if (sl->next == NULL) {
    // Dealing with local variable, just grab it from the current activation record
    sprintf(read_inst, "-%d(%%rbp), %s", WORD * (sl->offset + 2), out);
    ADD_INSTR("movq", read_inst);
  } else {
    // Dealing with a variable that's in a more global scope
    ADD_INSTR("push", "%rax");
    ADD_INSTR("movq", "-8(%rbp), %rax");
    for (link = sl; link->next && !link->next->is_mod; link = link->next) {
      ADD_INSTR("movq", "(%rax), %rax");
    }
    if (link->next && link->next->is_mod)
      sprintf(read_inst, "%d(%%rax), %s", WORD * (sl->offset + 1), out);
    else
      sprintf(read_inst, "-%d(%%rax), %s", WORD * (sl->offset + 1), out);
    ADD_INSTR("movq", read_inst);
    ADD_INSTR("pop", "%rax");
  }
  RETURN_BUFFER;
}

char* gen_assign_stmt(context_t ctx, assign_stmt_t assign) {
  CREATE_BUFFER;
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%r11");
  ADD_BLOCK(gen_expr(ctx, assign->assign->expr, "%r10"));
  ADD_BLOCK(gen_lval_expr(ctx, assign->lval, "%r11"));
  ADD_INSTR("movq", "%r10, (%r11)");
  ADD_INSTR("pop", "%r11");
  ADD_INSTR("pop", "%r10");
  RETURN_BUFFER;
}

char* gen_lval_expr(context_t ctx, expr_t lval, reg_t out) {
  static_link_t sl = NULL;
  char *read_inst = NULL;
  CREATE_BUFFER;
  switch(lval->kind) {
  case ID_EX:
    ADD_INSTR("push", "%rax");
    ADD_INSTR("leaq", "-8(%rbp), %rax");
    sl = ctx_get_id_offset(ctx, lval->u.id_ex);
    if (sl == NULL) {
      GEN_ERROR(concat("Cannot find l-value ", lval->u.id_ex));
    } else if (sl->levels > 0) {
      for (int i = 0; i < sl->levels; i++) {
        ADD_INSTR("movq", "(%rax), %rax");
      }
    }
    read_inst = malloc(64);
    sprintf(read_inst, "-%d(%%rax), ", WORD * (sl->offset + 1));
    ADD_INSTR("leaq", concat(read_inst, out));
    ADD_INSTR("pop", "%rax");
    break;
  case LITERAL_EX:
  case UNARY_EX:
  case BINARY_EX:
  case TERNARY_EX:
  case CALL_EX:
  case RANGE_EX:
  default:
    GEN_ERROR("INVALID LVAL");
    break;
  }
  RETURN_BUFFER;
}

char* gen_cond_stmt(context_t ctx, cond_stmt_t cond) {
  // TODO: This needs to generate morph stuff
  ll_t condition_labels = NULL;
  char *curr_label = NULL;
  char *end_label = NULL;
  CREATE_BUFFER;

  // Generate all of the condition labels
  for (cond_stmt_t c = cond; c; c = c->else_stmt ? c->else_stmt->u.cond_stmt : NULL) {
    if (condition_labels == NULL) {
      condition_labels = ll_new();
      condition_labels->val = gen_label("C");
      condition_labels->next = NULL;
    } else {
      ll_append(condition_labels, gen_label("C"));
    }
  }

  // Generate end label
  end_label = gen_label("C");

  // Add label, test condition, jump to end of conditional if necessary
  ll_t cl = condition_labels;
  for (cond_stmt_t c = cond; c; c = c->else_stmt ? c->else_stmt->u.cond_stmt : NULL) {
    if (c->test != NULL) {
      ADD_BLOCK(gen_expr(ctx, c->test, "%rax"));

      // TODO: This should be a morph
      ADD_INSTR("push", "%r10");
      ADD_INSTR("movq", "(%rax), %r10"); 
      ADD_INSTR("cmp", "$1, %r10");
      ADD_INSTR("pop", "%r10");
      ADD_INSTR("je", cl->val);
    } else {
      // NULL c->test means just assume the condition is true. This is because it was likely
      // an 'else' statement
      ADD_INSTR("jmp", cl->val);
    }
    cl = cl->next;
  }
  ADD_INSTR("jmp", end_label);

  // Condition labels w/ bodies
  cl = condition_labels;
  for (cond_stmt_t c = cond; c; c = c->else_stmt ? c->else_stmt->u.cond_stmt : NULL) {
    ADD_LABEL(cl->val);
    cl = cl->next;
    if (c->body != NULL) {
      for (stmt_t s = c->body; s; s = s->next) {
        ADD_BLOCK(gen_stmt(ctx, s));
      }
      ADD_INSTR("jmp", end_label);
    } else {
      ADD_INSTR("nop", NO_OPERANDS);
    }
  }

  // Add the end label
  ADD_LABEL(end_label);

  RETURN_BUFFER;
}

char* gen_loop_stmt(context_t ctx, loop_stmt_t loop) {
  char *loop_label = gen_label("L");
  char *break_label = gen_label("L");
  ctx_add_break_label(ctx, break_label, NULL);
  CREATE_BUFFER;
  ADD_LABEL(loop_label);
  for (stmt_t s = loop->body; s; s = s->next) {
    ADD_BLOCK(gen_stmt(ctx, s));
  }
  ADD_INSTR("jmp", loop_label);
  ADD_LABEL(break_label);
  ctx_pop_break_label(ctx);
  RETURN_BUFFER;
}

char* gen_break_stmt(context_t ctx, break_stmt_t brk) {
  CREATE_BUFFER;
  char *break_label = ctx_curr_break_label(ctx);
  if (break_label == NULL) {
    GEN_ERROR("No break label");
  }
  ADD_INSTR("jmp", break_label);
  RETURN_BUFFER;
}

char* gen_binary_expr(context_t ctx, binary_t binary, reg_t out) {
  CREATE_BUFFER;
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_BLOCK(gen_expr(ctx, binary->left, "%rdi"));
  ADD_BLOCK(gen_expr(ctx, binary->right, "%rsi"));
  switch(binary->op){
  case PLUS_OP:
    ADD_INSTR("call", "_add");
    break;
  case MINUS_OP:
    ADD_INSTR("call", "_sub");
    break;
  case MUL_OP:
    ADD_INSTR("call", "_mul");
    break;
  case DIV_OP:
    ADD_INSTR("call", "_div");
    break;
  case MOD_OP:
    ADD_INSTR("call", "_mod");
    break;
  case AND_OP:
    ADD_INSTR("call", "_and");
    break;
  case OR_OP:
    ADD_INSTR("call", "_or");
    break;
  case XOR_OP:
    ADD_INSTR("call", "_xor");
    break;
  case BIT_AND_OP:
    ADD_INSTR("call", "_b_and");
    break;
  case BIT_OR_OP:
    ADD_INSTR("call", "_b_or");
    break;
  case BIT_XOR_OP:
    ADD_INSTR("call", "_b_xor");
    break;
  case SHIFT_RIGHT_OP:
    ADD_INSTR("call", "_b_r_shift");
    break;
  case SHIFT_LEFT_OP:
    ADD_INSTR("call", "_b_l_shift");
    break;
  case EQ_EQ_OP:
    ADD_INSTR("call", "_eq_eq");
    break;
  case LT_OP:
    ADD_INSTR("call", "_lt");
    break;
  case GT_OP:
    ADD_INSTR("call", "_gt");
    break;
  case NOT_EQ_OP:
    ADD_INSTR("call", "_neq");
    break;
  case LT_EQ_OP:
    ADD_INSTR("call", "_lte");
    break;
  case GT_EQ_OP:
    ADD_INSTR("call", "_gte");
    break;
  default:
    GEN_ERROR("Unrecognized binary expression");
  }
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("movq", concat("%rax, ", out));
  RETURN_BUFFER;
}

char* gen_ternary_expr(context_t ctx, ternary_t ternary, reg_t out) {
  // TODO: More morph stuff is probably needed here
  char *middle_label = gen_label("T");
  char *right_label = gen_label("T");
  char *end_label = gen_label("T");
  CREATE_BUFFER;
  ADD_BLOCK(gen_expr(ctx, ternary->left, "%rax"));
  ADD_INSTR("movq", "(%rax), %rax");
  ADD_INSTR("cmp", "$1, %rax");
  ADD_INSTR("je", middle_label);
  ADD_INSTR("jmp", right_label);
  ADD_LABEL(middle_label);
  ADD_BLOCK(gen_expr(ctx, ternary->middle, out));
  ADD_INSTR("jmp", end_label);
  ADD_LABEL(right_label);
  ADD_BLOCK(gen_expr(ctx, ternary->right, out));
  ADD_LABEL(end_label);
  RETURN_BUFFER;
}

char* gen_unary_expr(context_t ctx, unary_t unary, reg_t out) {
  // TODO: More morph stuff is needed here
  CREATE_BUFFER;
  ADD_INSTR("push", "%rdi");
  ADD_BLOCK(gen_expr(ctx, unary->expr, "%rdi"));
  switch(unary->op) {
  case NOT_OP:
    ADD_INSTR("call", "_not");
    break;
  case BIT_NOT_OP:
    ADD_INSTR("call", "_b_not");
    break;
  case NEG_OP:
    ADD_INSTR("call", "_neg");
    break;
  case PRE_INC_OP:
    ADD_INSTR("call", "_pre_inc");
    break;
  case PRE_DEC_OP:
    ADD_INSTR("call", "_pre_dec");
    break;
  case POST_INC_OP:
    ADD_INSTR("call", "_post_inc");
    break;
  case POST_DEC_OP:
    ADD_INSTR("call", "_post_dec");
    break;
  default:
    GEN_ERROR("Unrecognized unary operator");
  }
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("movq", concat("%rax, ", out));
  RETURN_BUFFER;
}

char* codegen(root_t root, type_node_t *g) {
  graph = g;
  CREATE_BUFFER;
  ADD_INSTR(".global", "main");
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    context_t ctx = ctx_new();
    ctx_set_mod(ctx);
    ADD_BLOCK(gen_mod(ctx, mod));
  }

  // Generate main method 
  set_entrypoint(root);
  ADD_LABEL("main");
  ADD_INSTR("push", "%r15");
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_INSTR("call", "__module__Main_init");
  ADD_INSTR("movq", "%rax, %r15");
  ADD_INSTR("call", "Main_main");
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("pop", "%r15");

  // Generate data segment
  ADD_BLOCK(gen_data_segment());
  RETURN_BUFFER;
}
