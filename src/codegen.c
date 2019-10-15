// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"
#include "context.h"

/* MACROS */
#define WORD 8
#define NO_OPERANDS ""
#define CREATE_BUFFER char *buf = ""
#define RETURN_BUFFER return buf
#define PRINT_BUFFER printf("%s\n", buf)

#define ADD_BLOCK(B) if (strlen(B) > 0 && strcmp("\n", B) != 0) {    \
    buf = concat(buf, B);                                            \
    buf = concat(buf, "\n");                                         \
  }

#define ADD_LABEL(L) buf = concat(buf, L);      \
  buf = concat(buf, ":\n")

#define ADD_INSTR(I, O) buf = concat(buf, "\t");        \
  buf = concat(buf, I);                                 \
  buf = concat(buf, "\t");                              \
  buf = concat(buf, O);                                 \
  buf = concat(buf, "\n")

#define PUSH_CALLER_SAVES ADD_INSTR("push", "%r10");    \
  ADD_INSTR("push", "%r11");                            \
  ADD_INSTR("push", "%r12");                            \
  ADD_INSTR("push", "%r13");                            \
  ADD_INSTR("push", "%r14");                            \
  ADD_INSTR("push", "%r15");

#define POP_CALLER_SAVES ADD_INSTR("pop", "%r15");     \
  ADD_INSTR("pop", "%r14");                            \
  ADD_INSTR("pop", "%r13");                            \
  ADD_INSTR("pop", "%r12");                            \
  ADD_INSTR("pop", "%r11");                            \
  ADD_INSTR("pop", "%r10");


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
ll_t labels;
ll_t strings;
unsigned int curr_label = 0;
reg_t arg_registers[6] = {
                        "%rdi",
                        "%rsi",
                        "%rdx",
                        "%rcx",
                        "%r8",
                        "%r9"
};
char *entrypoint;

/* PROTOTYPES */
// Helper functions
void gen_error(char *msg);
unsigned int count_consts_and_vars(decl_t decl);
int get_id_offset(char *id, decl_t decl);
char* register_or_get_string_label(char *str);
void populate_decl_into_ctx(context_t ctx, decl_t decl);

// Generator functions
char* gen_label(char *label);
char* gen_data_segment();
char* gen_mod(mod_t mod);
char* gen_fun(context_t ctx, fun_decl_t fun);
char* gen_stmt(context_t ctx, stmt_t stmt);
char* gen_expr_stmt(context_t ctx, expr_stmt_t expr);
char* gen_expr(context_t ctx, expr_t expr, reg_t out);
char* gen_call_expr(context_t ctx, call_t call, reg_t out);
char* gen_literal_expr(context_t ctx, literal_t literal, reg_t out);
char* gen_id_expr(context_t ctx, char *id, reg_t out);
char* gen_assign_stmt(context_t ctx, assign_stmt_t assign);
char* gen_lval_expr(context_t ctx, expr_t lval, reg_t out);

/* HELPERS */
void gen_error(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

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
  for (const_decl_t c = decl->constants; c; c = c->next)
    ctx_add_constant(ctx, c->name);
  for (var_decl_t v = decl->vars; v; v = v->next)
    for (id_list_t id = v->names; id; id = id->next)
      ctx_add_variable(ctx, id->name);
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
  strings->val = str_label;
  str_label->str = str;
  str_label->label = gen_label("STR");
  ll_append(strings, str_label);
  return str_label->label;
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

char* gen_mod(mod_t mod) {
  CREATE_BUFFER;
  // Allocate module constants

  // Allocate module variables

  // Define type morph functions

  // Define functions
  for (fun_decl_t f = mod->decl->funs; f; f = f->next) {
    context_t ctx = ctx_new();
    buf = concat(buf, gen_fun(ctx, f));
  }
  RETURN_BUFFER;
}

char* gen_fun(context_t ctx, fun_decl_t fun) {
  // Calculate activation record space
  unsigned int preamble_space = count_consts_and_vars(fun->decl);
  for (arg_t arg = fun->args; arg; arg = arg->next) {
    preamble_space += WORD;
  }
  preamble_space += WORD; // Increment for static link
  char *space_str = concat("$", itoa(preamble_space));

  // Initialize activation record
  CREATE_BUFFER;
  ADD_LABEL(fun->name);
  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", concat(space_str, ", %rsp"));

  // Setup static link
  
  // Push arguments to stack
  unsigned int idx = 0;
  for (arg_t arg = fun->args; arg; arg = arg->next) {
    // TODO: Make sure it handles 6+ arguments correctly. This only does registers
    ctx_add_argument(ctx, arg->name);
    char *save_inst = malloc(32);
    sprintf(save_inst, "%s, %d(%%rbp)", arg_registers[idx], (idx + 1) * WORD);
    ADD_INSTR("movq", save_inst);
    idx++;
  }

  // Push constants and variables to stack
  populate_decl_into_ctx(ctx, fun->decl);
  for (const_decl_t c = fun->decl->constants; c; c = c-> next) {
    // TODO: Handle the different kinds of assignment
    ADD_BLOCK(gen_expr(ctx, c->assign->expr, "%rax"));
    char *save_inst = malloc(32);
    sprintf(save_inst, "%%rax, -%d(%%rbp)", (idx + 1) * WORD);
    ADD_INSTR("movq", save_inst);
    idx++;
  }

  for (var_decl_t v = fun->decl->vars; v; v = v->next) {
    // TODO: Handle the different kinds of assignment
    expr_t expr = v->assign->expr;
    for (id_list_t id = v->names; id; id = id->next) {
      ADD_BLOCK(gen_expr(ctx, expr, "%rax"));
      char *save_inst = malloc(32);
      sprintf(save_inst, "%%rax, -%d(%%rbp)", (idx + 1) * WORD);
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
    // TODO
    break;
  case FOR_STMT:
    // TODO
    break;
  case LOOP_STMT:
    // TODO
    break;
  case CASE_STMT:
    // TODO
    break;
  case ASSIGN_STMT:
    ADD_BLOCK(gen_assign_stmt(ctx, stmt->u.assign_stmt));
    break;
  default:
    // TODO
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
    // TODO
    ADD_BLOCK(gen_literal_expr(ctx, expr->u.literal_ex, out));
    break;
  case UNARY_EX:
    // TODO
    break;
  case BINARY_EX:
    // TODO
    break;
  case TERNARY_EX:
    // TODO
    break;
  case CALL_EX:
    ADD_BLOCK(gen_call_expr(ctx, expr->u.call_ex, out));
    break;
  case RANGE_EX:
    // TODO
    break;
  default:
    // TODO
    break;
  }
  RETURN_BUFFER;
}

char* gen_call_expr(context_t ctx, call_t call, reg_t out) {
  int arg_idx = 0;
  expr_list_t curr_arg = call->args;
  CREATE_BUFFER;
  // PUSH_CALLER_SAVES;

  // Put arguments in registers
  while (curr_arg) {
    ADD_INSTR("push", arg_registers[arg_idx]);
    ADD_BLOCK(gen_expr(ctx, curr_arg->expr, arg_registers[arg_idx]));
    curr_arg = curr_arg->next;
    arg_idx++;
  }

  // Make call to function
  ADD_INSTR("call", call->id);

  // Restore argument registers
  for (int i = arg_idx - 1; i >= 0; i--) {
    ADD_INSTR("pop", arg_registers[i]);
  }

  // POP_CALLER_SAVES;
  printf("Out reg %s\n", out);
  if (strcmp("%rax", out) != 0) {
    ADD_INSTR("movq", concat("%rax, ", out));
  }
  RETURN_BUFFER;
}

char* gen_literal_expr(context_t ctx, literal_t literal, reg_t out) {
  char *str_label;
  char *int_literal;
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
    // TODO
    break;
  case BOOLEAN_LIT:
    // TODO
    break;
  default:
    // TODO
    break;
  }
  RETURN_BUFFER;
}

char* gen_data_segment() {
  if (strings == NULL) return "";
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
  // TODO: Add static link traversal
  CREATE_BUFFER;
  static_link_t sl = ctx_get_id(ctx, id);
  char *read_inst = malloc(64);
  sprintf(read_inst, "-%d(%%rbp), %s", WORD * (sl->offset + 1), out);
  ADD_INSTR("movq", read_inst);
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
  static_link_t sl;
  char *read_inst;
  CREATE_BUFFER;
  switch(lval->kind) {
  case ID_EX:
    sl = ctx_get_id(ctx, lval->u.id_ex);
    read_inst = malloc(64);
    sprintf(read_inst, "-%d(%%rbp), ", WORD * (sl->offset + 1));
    ADD_INSTR("leaq", concat(read_inst, out));
    break;
  case LITERAL_EX:
  case UNARY_EX:
  case BINARY_EX:
  case TERNARY_EX:
  case CALL_EX:
  case RANGE_EX:
  default:
    // TODO: This should error
    break;
  }
  RETURN_BUFFER;
}

char* codegen(root_t root) {
  CREATE_BUFFER;
  ADD_BLOCK("\t.global main\n");
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    ADD_BLOCK(gen_mod(mod));
  }
  ADD_BLOCK(gen_data_segment());
  RETURN_BUFFER;
}
