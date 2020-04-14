// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"
#include "context.h"
#include "morph_graph.h"

/* MACROS */
#define WORD 8
#define MODULE_MIN_SIZE 16
#define NO_OPERANDS ""
#define EMPTY_BUFFER ""
#define CREATE_BUFFER char *buf = ""
#define RETURN_BUFFER return buf
#define PRINT_BUFFER printf("%s\n", buf)

#define INT_LITERAL(I) (concat("$", itoa(I)))

#define ADD_BLOCK(B) \
  do { \
    buf = concat(buf, B);      \
    buf = concat(buf, "\n");   \
  } while(0)

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

#define GEN_ENABLE_TYPE(T) activate_node(graph, T);
#define GEN_DISABLE_TYPE(T) deactivate_node(graph, T);

/* TYPES */
typedef struct label_st        *label_t;
typedef struct string_label_st *string_label_t;
typedef struct file_no_st      *file_no_t;
typedef char                   *reg_t;

struct label_st {
  char         *label;
  unsigned int  count;
};

struct string_label_st {
  char    *str;
  char    *label;
};

struct file_no_st {
  char *file_name;
  unsigned number;
};

/* GLOBALS */
type_node_t *graph;
ll_t labels = NULL;
ll_t strings = NULL;
ll_t file_no_map = NULL;
unsigned int curr_label = 0;
unsigned int curr_fileno = 1;
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
bool has_main(root_t root);
void build_file_no_map(root_t root);
unsigned get_file_no(char *file_name);

// Generator functions
char* gen_label(char *label);
char* gen_data_segment();
char* gen_type_graph_segment();
char* gen_mod(context_t ctx, mod_t mod);
char* gen_fun(context_t ctx, fun_decl_t fun);
char* gen_type(context_t ctx, type_decl_t type);
char* gen_type_constructor(context_t ctx, type_decl_t type);
char* gen_morph(context_t ctx, char *type_name, morph_t morph);
char* gen_identity_morph(char *src, char *dest);
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
char* gen_manage_types(type_decl_t t, int enable);
char* gen_try_catch_stmt(context_t ctx, try_stmt_t try_catch);
char* gen_throw_stmt(context_t ctx, throw_stmt_t throw);
char* gen_array_dimensions(context_t ctx, expr_list_t dimensions, reg_t out);

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
	for (const_decl_t c = decl->constants; c; c = c->next) {	
		//ctx_add_constant(ctx, c->name, c->ty->u.name_ty);
		switch (c->ty->kind) {
    	case NAME_TY:
				ctx_add_constant(ctx, c->name, c->ty->u.name_ty);
				break;
			case ARRAY_TY:
				ctx_add_constant(ctx, c->name, "array");
				break;
			case REC_TY:
				ctx_add_constant(ctx, c->name, "record");
				break;
			case ENUM_TY:
				ctx_add_constant(ctx, c->name, "enum");
				break;
			case MORPH_TY:
				ctx_add_constant(ctx, c->name, "morph");
				break;
			default: 
				GEN_ERROR("unrecognized constant type");
				break;
		}
	}
  for (var_decl_t v = decl->vars; v; v = v->next) {
    for (id_list_t id = v->names; id; id = id->next) {
      //ctx_add_variable(ctx, id->name, v->type->u.name_ty);
			switch (v->type->kind) {
    		case NAME_TY:
					ctx_add_variable(ctx, id->name, v->type->u.name_ty);
					break;
				case ARRAY_TY:
					ctx_add_variable(ctx, id->name, "array");
					break;
				case REC_TY:
					ctx_add_variable(ctx, id->name, "record");
					break;
				case ENUM_TY:
					ctx_add_variable(ctx, id->name, "enum");
					break;
				case MORPH_TY:
					ctx_add_variable(ctx, id->name, "morph");
					break;
				default: 
					GEN_ERROR("unrecognized variable type");
					break;
			}
		}	
	}
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

void build_file_no_map(root_t root) {
  unsigned curr = 1;
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    file_no_t file_no = malloc(sizeof(struct file_no_st));
    file_no->file_name = mod->file_name;
    file_no->number = curr;
      curr++;
    if (!file_no_map) {
      file_no_map = ll_new();
      file_no_map->val = file_no;
    } else {
      ll_append(file_no_map, file_no);
    }
  }
}

unsigned get_file_no(char *file_name) {
  for (ll_t curr = file_no_map; curr; curr = curr->next) {
    file_no_t file_no = (file_no_t) curr->val;
    if (strcmp(file_name, file_no->file_name) == 0)
      return file_no->number;
  }
  char *file_no_err;
  asprintf(&file_no_err, "Cannot find mapping for file name %s", file_name);
  GEN_ERROR(file_no_err);
  return 0;
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

char* gen_array_dimensions(context_t ctx, expr_list_t dimensions, reg_t out) {
  CREATE_BUFFER;
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_INSTR("push", "%rdx");
  int num_dims = 0;
  int idx = 0;
  expr_list_t curr_dim = dimensions;
  // get number of dimensions
  for (; curr_dim; curr_dim = curr_dim->next)
    num_dims++;
  //allocate space for the dimensions array
  ADD_INSTR("movq", concat(concat("$", itoa(num_dims)), ", %rdi"));
  ADD_INSTR("call", "alloc_array");
  ADD_INSTR("movq", "%rax, %rdi");
  // populate array with dimensions
  curr_dim = dimensions;
  for (; curr_dim; curr_dim = curr_dim->next) {
    ADD_BLOCK(gen_expr(ctx, curr_dim->expr, "%rsi"));
    ADD_INSTR("movq", concat(concat("$", itoa(idx)), ", %rdx"));
    ADD_INSTR("call", "__set_data_member");
    idx++;
  }
  ADD_INSTR("pop", "%rdx");
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("movq", concat("%rax, ", out));

  RETURN_BUFFER;
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

  for (type_decl_t t = mod->decl->types; t; t = t->next)
    ctx_add_type(ctx, t);

  for (type_decl_t t = mod->decl->types; t; t = t->next)
    ADD_BLOCK(gen_type(ctx, t));

  // Allocate module block
  char *module_label = concat("__module__", concat(ctx_get_scope_name(ctx), "_init"));
  ADD_INSTR(".globl", module_label);
  char *type_directive = NULL;
  asprintf(&type_directive, "%s, @function", module_label);
  ADD_INSTR(".type", type_directive);
  ADD_LABEL(module_label);

  if (ctx_is_global(ctx)) {
    char *dwarf_file_directive;
    asprintf(&dwarf_file_directive, ".file %d \"%s\"", get_file_no(mod->file_name)+1, mod->file_name);
    ADD_INSTR(dwarf_file_directive, NO_OPERANDS);
  }

  // Populate module block with constants and variables
  int mod_size = MODULE_MIN_SIZE;
  for (const_decl_t c = mod->decl->constants; c; c = c->next) {
    mod_size += WORD;
  }
  for (var_decl_t v = mod->decl->vars; v; v = v->next) {
    // TODO: This is incorrect and should loop over the id list
    mod_size += WORD;
  }
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("movq", concat(INT_LITERAL(mod_size), ", %rdi"));
  ADD_INSTR("call", "malloc");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("movq", "%rax, %r10");
  ADD_INSTR("push", "%r10");

  int offset = 1;
  for (const_decl_t c = mod->decl->constants; c; c = c->next) {
    assign_t assign = c->assign;
    ADD_INSTR("push", "%r10");
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("push", "%rsi");
    ADD_BLOCK(gen_expr(ctx, assign->expr, "%rdi"));
    char *type_label = NULL;
    switch(c->ty->kind) {
    case NAME_TY:
      type_label = register_or_get_string_label(c->ty->u.name_ty);
      break;
    case ARRAY_TY:
      // need to revisit to handle typed arrays
      type_label = register_or_get_string_label("array");
      break;
    case REC_TY:
      type_label = register_or_get_string_label("record");
      break;
    case ENUM_TY:
      type_label = register_or_get_string_label("enum");
      break;
    default:
      GEN_ERROR("Unrecognized variable type");
      break;
    }
    ADD_INSTR("movq", concat(concat("$", type_label), ", %rsi"));
    ADD_INSTR("call", "__morph");
    ADD_INSTR("pop", "%rsi");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("pop", "%r10");
    ADD_INSTR("movq", concat("%rax, ", concat(itoa(offset * WORD), "(%r10)")));
    offset++;
  }
  for (var_decl_t v = mod->decl->vars; v; v = v->next) {
    ADD_INSTR("push", "%r10");
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("push", "%rsi");
    if (v->assign) {
      assign_t assign = v->assign;
      ADD_BLOCK(gen_expr(ctx, assign->expr, "%rdi"));
    }
    char *type_label = NULL;
    // variables used for array stuff
    char *array_type = NULL; // type of array (if typed)
    int num_dims = 0; // number of dimensions in array
    int *dims = NULL; // array dimensions
    expr_list_t curr_dim = NULL; // used for looping through
    switch(v->type->kind) {
    case NAME_TY:
      type_label = register_or_get_string_label(v->type->u.name_ty);
      break;
    case ARRAY_TY:
      // check if array is typed
      if (v->type->u.array_ty->type) {
        array_type = concat("array of ", v->type->u.array_ty->type->u.name_ty);
        type_label = register_or_get_string_label(array_type);
        ADD_INSTR("movq", concat(concat("$", type_label), ", %rdi"));
        ADD_INSTR("call", "__add_type");
      } else {
        type_label = register_or_get_string_label("array");
      }
      // if the the array was declared but not initialized, then allocate space for it 
      if (v->type->u.array_ty->dimensions && !v->assign) {
        ADD_BLOCK(gen_array_dimensions(ctx, v->type->u.array_ty->dimensions, "%rdi"));
        ADD_INSTR("movq", concat(concat("$", type_label), ", %rsi"));
        ADD_INSTR("call", "init_default_array");
        ADD_INSTR("movq", "%rax, %rdi");
      }
      break;
    case REC_TY:
      type_label = register_or_get_string_label("record");
      break;
    case ENUM_TY:
      type_label = register_or_get_string_label("enum");
      break;
    default:
      GEN_ERROR("Unrecognized variable type");
      break;
    }
    ADD_INSTR("movq", concat(concat("$", type_label), ", %rsi"));
    ADD_INSTR("call", "__morph");
    ADD_INSTR("pop", "%rsi");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("pop", "%r10");
    ADD_INSTR("movq", concat("%rax, ", concat(itoa(offset * WORD), "(%r10)")));
    offset++;
  }

  // Enable all types defined in this scope
  ADD_BLOCK(gen_manage_types(mod->decl->types, 1));

  // Generate the init block
  if (mod->stmts != NULL)
    for (stmt_t s = mod->stmts; s; s = s->next)
      ADD_BLOCK(gen_stmt(ctx, s));

  // Disable all types defined in this scope
  ADD_BLOCK(gen_manage_types(mod->decl->types, 0));

  ADD_INSTR("pop", "%r10");
  ADD_INSTR("movq", "%r10, %rax");
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("ret", NO_OPERANDS);

  // Define sub modules

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
  if (fun->is_extern) {
    RETURN_BUFFER;
  }
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

  for (type_decl_t t = fun->decl->types; t; t = t->next)
    ctx_add_type(ctx, t);
  
  for (type_decl_t t = fun->decl->types; t; t = t->next)
    ADD_BLOCK(gen_type(ctx, t));
  
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
  ADD_INSTR(".cfi_startproc", NO_OPERANDS);
  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", concat(space_str, ", %rsp"));

  // Setup static link
  ADD_INSTR("movq", "%r10, -8(%rbp)");
  
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
    ADD_INSTR("push", "%r10");
    ADD_INSTR("push", "%rdi");
    ADD_INSTR("push", "%rsi");
    ADD_BLOCK(gen_expr(ctx, c->assign->expr, "%rdi"));
    char *type_label = register_or_get_string_label(c->ty->u.name_ty);
    ADD_INSTR("movq", concat(concat("$", type_label), ", %rsi"));
    ADD_INSTR("call", "__morph");
    ADD_INSTR("pop", "%rsi");
    ADD_INSTR("pop", "%rdi");
    ADD_INSTR("pop", "%r10");
    char *save_inst = malloc(64);
    sprintf(save_inst, "%%rax, -%d(%%rbp)", (idx + 1) * WORD);
    ADD_INSTR("movq", save_inst);
    idx++;
  }

  for (var_decl_t v = fun->decl->vars; v; v = v->next) {
    // TODO: Handle the different kinds of assignment
    expr_t expr = v->assign->expr;
    for (id_list_t id = v->names; id; id = id->next) {
      ADD_INSTR("push", "%r10");
      ADD_INSTR("push", "%rdi");
      ADD_INSTR("push", "%rsi");
      ADD_BLOCK(gen_expr(ctx, expr, "%rdi"));
      char *type_label = register_or_get_string_label(v->type->u.name_ty);
      ADD_INSTR("movq", concat(concat("$", type_label), ", %rsi"));
      ADD_INSTR("call", "__morph");
      ADD_INSTR("pop", "%rsi");
      ADD_INSTR("pop", "%rdi");
      ADD_INSTR("pop", "%r10");
      char *save_inst = concat("%rax, -", concat(itoa((idx+1)*WORD), "(%rbp)"));
      ADD_INSTR("movq", save_inst);
      idx++;
    }
  }

  // Enable all types defined in this scope
  ADD_BLOCK(gen_manage_types(fun->decl->types, 1));

  // Start executing statements
  for (stmt_t s = fun->stmts; s; s = s->next) {
    ADD_BLOCK(gen_stmt(ctx, s));
  }

  // Disable all types defined in this scope
  ADD_BLOCK(gen_manage_types(fun->decl->types, 0));

  // Cleanup activation record
  ADD_INSTR("movq", "$0, %rax");
  ADD_INSTR("leave", NO_OPERANDS);
  ADD_INSTR("ret", NO_OPERANDS);
  ADD_INSTR(".cfi_endproc", NO_OPERANDS);
  RETURN_BUFFER;
}

char* gen_stmt(context_t ctx, stmt_t stmt) {
  CREATE_BUFFER;
  char *loc_directive = NULL;
  asprintf(&loc_directive, ".loc %d %d", curr_fileno, stmt->pos->line_no);
  ADD_INSTR(loc_directive, NO_OPERANDS);
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
  case TRY_STMT:
    ADD_BLOCK(gen_try_catch_stmt(ctx, stmt->u.try_stmt));
    break;
  case THROW_STMT:
    ADD_BLOCK(gen_throw_stmt(ctx, stmt->u.throw_stmt));
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
  char *idx = NULL;
  switch(expr->kind) {
  case ID_EX:
    // check expression for accessors
    if (expr->accessors) {
      ADD_BLOCK(gen_array_dimensions(ctx, expr->accessors->u.subscript_expr, "%rdi"));
      ADD_BLOCK(gen_id_expr(ctx, expr->u.id_ex, "%rsi"));
      ADD_INSTR("push", "%rsi");
      ADD_INSTR("call", "calc_index");
      ADD_INSTR("pop", "%rsi");
      ADD_INSTR("movq", "%rsi, %rdi");
      ADD_INSTR("movq", "%rax, %rsi");
      ADD_INSTR("call", "__get_data_member");
      ADD_INSTR("movq", concat("%rax, ", out));
    } 
    else
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
    reg_t curr_reg = arg_registers[arg_idx];
    ADD_INSTR("push", curr_reg);
    ADD_BLOCK(gen_expr(ctx, curr_arg->expr, "%rax"));
    if (args) {
      char *arg_type_label = register_or_get_string_label(args->type->u.name_ty);
      ADD_INSTR("push", "%r10");
      ADD_INSTR("push", "%rdi");
      ADD_INSTR("push", "%rsi");
      ADD_INSTR("movq", "%rax, %rdi");
      ADD_INSTR("movq", concat(concat("$", arg_type_label), ", %rsi"));
      ADD_INSTR("call", "__morph");
      ADD_INSTR("pop", "%rsi");
      ADD_INSTR("pop", "%rdi");
      ADD_INSTR("pop", "%r10");
    }

    ADD_INSTR("movq", concat("%rax, ", curr_reg));
    
    arg_idx++;
    if (args != NULL)
      args = args->next;
  }

  // Pass static link
  ADD_INSTR("push", "%r10");

  // TODO: Only change the static link if function is inner function 

  // Make call to function
  func_link_t fl = ctx_get_function(ctx, call->id);
  if (fl == NULL) {
    // TODO: This basically means if we can't find the function, just call the name
    ADD_INSTR("call", call->id);
  } else {
    ADD_INSTR("leaq", "-8(%rbp), %r10");
    for (int i = 0; i < fl->levels; i++) {
      ADD_INSTR("movq", "(%r10), %r10");
    }  
    ADD_INSTR("call", fl->id);
  }

  // Restore argument registers
  ADD_INSTR("pop", "%r10");
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
  // for use with array literal
  int len = 0; // length of array
  int i = 0; // index in array
  expr_list_t curr = NULL;
  CREATE_BUFFER;
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_INSTR("push", "%r10");
  switch(literal->kind) {
  case STRING_LIT:
    str_label = concat("$", register_or_get_string_label(literal->u.string_lit));
    ADD_INSTR("movq", concat(str_label, ", %rdi"));
    ADD_INSTR("call", "alloc_str");
    break;
  case INTEGER_LIT:
    int_literal = concat("$", literal->u.integer_lit);
    ADD_INSTR("movq", concat(int_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_int");
    break;
  case REAL_LIT:
    real_literal = malloc(64);
    sprintf(real_literal, "$%lu", f_to_int(literal->u.real_lit));
    ADD_INSTR("movq", concat(real_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_real");
    break;
  case BOOLEAN_LIT:
    bool_literal = concat("$", itoa(literal->u.bool_lit == TRUE_BOOL));
    ADD_INSTR("movq", concat(bool_literal, ", %rdi"));
    ADD_INSTR("call", "alloc_bool");
    break;
  case ARRAY_LIT:
    // get length of array
    curr = literal->u.array_lit;
    for (; curr; curr = curr->next)
      len++;
    // store length of array in %rdi
    ADD_INSTR("movq", concat("$", concat(itoa(len), ", %rdi")));
    // allocate space for array
    ADD_INSTR("call", "alloc_array");
    ADD_INSTR("pop", "%r10");
    // move the pointer to the array to %rdi
    ADD_INSTR("movq", "%rax, %rdi"); 
    // populate array 
    curr = literal->u.array_lit;
    for (; curr; curr = curr->next) {
      ADD_BLOCK(gen_expr(ctx, curr->expr, "%rsi"));
      ADD_INSTR("movq", concat("$", concat(itoa(i), ", %rdx")));
      ADD_INSTR("call", "__set_data_member");
      i++;
    }
    ADD_INSTR("movq", "%rdi, %rax"); 
    ADD_INSTR("push", "%r10");
    break;
  case NULL_LIT:
    ADD_INSTR("movq", concat("$0, ", out));
    break;
  default:
    GEN_ERROR("Unknown literal");
  }
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("movq", concat("%rax, ", out));
  RETURN_BUFFER;
}

char* gen_data_segment() {
  CREATE_BUFFER;
  ADD_INSTR(".section", ".data");
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
  if (sl->is_mod != 1 && sl->next == NULL) {
    // Dealing with local variable, just grab it from the current activation record
    sprintf(read_inst, "-%d(%%rbp), %s", WORD * (sl->offset + 2), out);
    ADD_INSTR("movq", read_inst);
  } else if (sl->is_mod) {
    // We are directly in a module's scope, which means we are probably an init block
    sprintf(read_inst, "%d(%%r10), %s", WORD * (sl->offset + 1), out);
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
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_INSTR("push", "%rdx");
  // if assignment dest is an array member call __assign_array_member()
  // else call __assign_simple()
  if (assign->lval->accessors) {
    ADD_BLOCK(gen_lval_expr(ctx, assign->lval, "%rdx"));
    ADD_INSTR("push", "%rdx");
    ADD_BLOCK(gen_array_dimensions(ctx, assign->lval->accessors->u.subscript_expr, "%rsi"));
    ADD_BLOCK(gen_expr(ctx, assign->assign->expr, "%rdi"));
    //ADD_INSTR("pop", "%rsi");
    ADD_INSTR("pop", "%rdx");
    ADD_INSTR("call", "__assign_array_member");
  } else{
    ADD_BLOCK(gen_lval_expr(ctx, assign->lval, "%rsi"));
    ADD_BLOCK(gen_expr(ctx, assign->assign->expr, "%rdi"));
    ADD_INSTR("call", "__assign_simple");
  }
  ADD_INSTR("pop", "%rdx");
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("pop", "%r10");
  RETURN_BUFFER;
}

char* gen_lval_expr(context_t ctx, expr_t lval, reg_t out) {
  static_link_t sl = NULL;
  static_link_t link = NULL;
  char read_inst[64];
  char *idx = NULL;
  CREATE_BUFFER;
  switch(lval->kind) {
  case ID_EX:
    sl = ctx_get_id_offset(ctx, lval->u.id_ex);
    if (sl == NULL) {
      GEN_ERROR(concat("Cannot find l-value ", lval->u.id_ex)); 
    }

    if (sl->is_mod != 1 && sl->next == NULL) {
      // Dealing with local variable, just grab it from the current activation record
      sprintf(read_inst, "-%d(%%rbp), %s", WORD * (sl->offset + 2), out);
      ADD_INSTR("movq", read_inst);
    } else if (sl->is_mod) {
      // We are directly in a module's scope, which means we are probably an init block
      sprintf(read_inst, "%d(%%r10), %s", WORD * (sl->offset + 1), out);
      ADD_INSTR("movq", read_inst);
    } else {
      // Dealing with a variable that's in a more global scope
      ADD_INSTR("movq", "-8(%rbp), %rax");
      for (link = sl; link->next && !link->next->is_mod; link = link->next) {
        ADD_INSTR("movq", "(%rax), %rax");
      }
      if (link->next && link->next->is_mod)
        sprintf(read_inst, "%d(%%rax), %s", WORD * (sl->offset + 1), out);
      else
        sprintf(read_inst, "-%d(%%rax), %s", WORD * (sl->offset + 1), out);
      ADD_INSTR("movq", read_inst);
    }

    /* /\* if (sl == NULL) { *\/ */
    /* /\*   GEN_ERROR(concat("Cannot find l-value ", lval->u.id_ex)); *\/ */
    /* /\* } else if (sl->levels > 0) { *\/ */
    /* /\*   for (int i = 0; i < sl->levels; i++) { *\/ */
    /* /\*     ADD_INSTR("movq", "(%rax), %rax"); *\/ */
    /* /\*   } *\/ */
    /* /\* } *\/ */
    /* sprintf(read_inst, "-%d(%%rax), ", WORD * (sl->offset + 1)); */
    /* ADD_INSTR("leaq", concat(read_inst, out)); */
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
  char *bool_label = register_or_get_string_label("boolean");
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

      // TODO: This should be a morph
      ADD_INSTR("push", "%r10");
      ADD_INSTR("push", "%rdi");
      ADD_INSTR("push", "%rsi");
      ADD_BLOCK(gen_expr(ctx, c->test, "%rdi"));
      ADD_INSTR("movq", concat(concat("$", bool_label), ", %rsi"));
      ADD_INSTR("call", "__morph");
      ADD_INSTR("movq", "%rax, %rdi");
      ADD_INSTR("movq", concat(INT_LITERAL(0), ", %rsi"));
      ADD_INSTR("call", "__get_data_member");
      ADD_INSTR("cmp", "$1, %rax");
      ADD_INSTR("pop", "%rsi");
      ADD_INSTR("pop", "%rdi");
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
  ADD_INSTR("push", "%r10");
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
  ADD_INSTR("pop", "%r10");
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

char* gen_text_segment(root_t root) {
  CREATE_BUFFER;
  char *curr_file = root->mods->file_name;
  char *file_directive;
  asprintf(&file_directive, "\"%s\"", curr_file);
  ADD_INSTR(".file", file_directive);
  ADD_INSTR(".section", ".text");
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    if (strcmp(mod->file_name, curr_file) != 0) {
      curr_file = mod->file_name;
      asprintf(&file_directive, "\"%s\"", curr_file);
      ADD_INSTR(".file", file_directive);
    }
    context_t ctx = ctx_new();
    ctx_set_mod(ctx);
    ctx_set_global(ctx, true);
    ADD_BLOCK(gen_mod(ctx, mod));
  }
  if (has_main(root)) {
    ADD_INSTR(".global", "main");
    // Generate main method 
    ADD_LABEL("main");
    ADD_INSTR("call", "init_type_graph");
    ADD_INSTR("push", "%r10");
    ADD_INSTR("call", "__module__Main_init");
    ADD_INSTR("movq", "%rax, %r10");
    ADD_INSTR("pop", "%r10");
    ADD_INSTR("ret", NO_OPERANDS);
  }
  RETURN_BUFFER;
}

char* gen_type(context_t ctx, type_decl_t type) {
  char *type_label = concat("__type__", type->name);
  char *type_name_label = register_or_get_string_label(type->name);
  char *parent_name_label = register_or_get_string_label(type->type->u.name_ty);
  int morph_count = 0;
  for (morph_t morph = type->morphs; morph; morph = morph->next)
    morph_count++;
  CREATE_BUFFER;
  ADD_INSTR(".section", ".data");
  ADD_LABEL(type_label);
  ADD_INSTR(".quad", type_name_label);
  ADD_INSTR(".quad", parent_name_label);
  ADD_INSTR(".quad", concat("__morph__", concat(type->type->u.name_ty, concat("__", type->name))));
  ADD_INSTR(".quad", concat("__morph__", concat(type->name, concat("__", type->type->u.name_ty))));
  ADD_INSTR(".quad", itoa(morph_count));
  for (morph_t morph = type->morphs; morph; morph = morph->next) {
    char *target_label = register_or_get_string_label(morph->target);
    ADD_INSTR(".quad", target_label);
    ADD_INSTR(".quad", concat("__morph__", concat(type->name, concat("__", morph->target))));
  }
  ADD_INSTR(".section", ".type_graph");
  ADD_INSTR(".quad", type_label);
  ADD_INSTR(".section", ".text");
  ADD_BLOCK(gen_identity_morph(type->name, type->type->u.name_ty));
  ADD_BLOCK(gen_identity_morph(type->type->u.name_ty, type->name));
  ADD_BLOCK(gen_type_constructor(ctx, type));
  for (morph_t morph = type->morphs; morph; morph = morph->next) {
    ADD_BLOCK(gen_morph(ctx, type->name, morph));
  }
  RETURN_BUFFER;
}

char* gen_type_constructor(context_t ctx, type_decl_t type) {
  // TODO: This assumes no morph chains as the "base type". Also doesn't work for non-primitive types
  CREATE_BUFFER;
  ADD_LABEL(concat("__type__constructor__", type->name));
  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("push", "%r10");
  if (strcmp(type->type->u.name_ty, "integer") == 0) {
    ADD_INSTR("movq", "$0, %rdi");
    ADD_INSTR("call", "alloc_int");
  } else if (strcmp(type->type->u.name_ty, "string") == 0) {
    char *str_label = register_or_get_string_label("");
    ADD_INSTR("movq", concat("$", concat(str_label, ", %rdi")));
    ADD_INSTR("call", "alloc_string");
  } else if (strcmp(type->type->u.name_ty, "real") == 0) {
    ADD_INSTR("movq", "$0, %rdi");
    ADD_INSTR("call", "alloc_real");
  } else if (strcmp(type->type->u.name_ty, "boolean") == 0) {
    ADD_INSTR("movq", "$1, %rdi");
    ADD_INSTR("call", "alloc_bool");
  } else {
    ADD_INSTR("call", concat("__type__constructor__", type->type->u.name_ty));
  }
  ADD_INSTR("movq", "%rax, %rdi");
  ADD_INSTR("call", concat("__morph__", concat(type->name, concat("__", type->type->u.name_ty))));
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("leave", NO_OPERANDS);
  ADD_INSTR("ret", NO_OPERANDS);
  RETURN_BUFFER;
}

char* gen_identity_morph(char *src, char *dest) {
  char *dest_label = register_or_get_string_label(dest);
  CREATE_BUFFER;
  ADD_LABEL(concat("__morph__", concat(src, concat("__", dest))));
  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("push", "%r10");
  ADD_INSTR("movq", concat(concat("$", dest_label), ", %rsi"));
  ADD_INSTR("call", "__identity_helper");
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("leave", NO_OPERANDS);
  ADD_INSTR("ret", NO_OPERANDS);
  RETURN_BUFFER;
}

char* gen_manage_types(type_decl_t types, int enable) {
  CREATE_BUFFER;
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%rdi");
  for (type_decl_t t = types; t; t = t->next) {
    ADD_INSTR("movq", concat(concat("$", register_or_get_string_label(t->name)), ", %rdi"));
    ADD_INSTR("call", enable ? "__activate_type" : "__deactivate_type");
  }
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("pop", "%r10");
  RETURN_BUFFER;
}

char* gen_morph(context_t ctx, char *type_name, morph_t morph) {
  CREATE_BUFFER;
  ADD_LABEL(concat("__morph__", concat(type_name, concat("__", morph->target))));

  /* TODO: Allocate space for "this" and "that" or whatever variables 
   * we decided on. This will probably require creating a new context.
   */
  context_t new_ctx = ctx_new();
  ctx_set_parent(new_ctx, ctx);
  ctx_add_variable(new_ctx, "this", type_name);
  ctx_add_variable(new_ctx, "that", morph->target);

  ADD_INSTR("push", "%rbp");
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", "$16, %rsp");
  ADD_INSTR("push", "%r10");

  // Prepare "this" variable
  ADD_INSTR("call", concat("__type__constructor__", type_name));
  ADD_INSTR("movq", "%rax, -8(%rbp)");

  type_decl_t target_type_decl = (type_decl_t) ctx_get_type(ctx, morph->target);
  if (target_type_decl == NULL) {
    GEN_ERROR(concat("Cannot find declaration for type ", morph->target)); 
  }
  // Prepare "that" variable
  if (strcmp(target_type_decl->type->u.name_ty, "integer") == 0) {
    ADD_INSTR("movq", "$0, %rdi");
    ADD_INSTR("call", "alloc_integer");
  } else if (strcmp(target_type_decl->type->u.name_ty, "string") == 0) {
    char *str_label = register_or_get_string_label("");
    ADD_INSTR("movq", concat("$", concat(str_label, ", %rdi")));
    ADD_INSTR("call", "alloc_string");
  } else if (strcmp(target_type_decl->type->u.name_ty, "real") == 0) {
    ADD_INSTR("movq", "$0, %rdi");
    ADD_INSTR("call", "alloc_real");
  } else if (strcmp(target_type_decl->type->u.name_ty, "boolean") == 0) {
    ADD_INSTR("movq", "$1, %rdi");
    ADD_INSTR("call", "alloc_bool");
  } else {
    ADD_INSTR("call", concat("__type__constructor__", morph->target));
  }
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("movq", "%rax, -16(%rbp)");
  for (stmt_t s = morph->defn; s; s = s->next) {
    ADD_BLOCK(gen_stmt(new_ctx, s));
  }
  ADD_INSTR("movq", "-8(%rbp), %rax");
  ADD_INSTR("ret", NO_OPERANDS);
  RETURN_BUFFER;
}

char* gen_try_catch_stmt(context_t ctx, try_stmt_t try_catch) {
  char *catch_label = gen_label("CATCH");
  char *end_label = gen_label("TRY_END");
  CREATE_BUFFER;
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%rdi");

  // Initialize the libunwind cursor
  ADD_INSTR("call", "_init_try");
  ADD_INSTR("movq", "%rax, %rdi");

  // Save the previously push %r10 and %rdi registers into other registers,
  // that way getcontext will save them off
  ADD_INSTR("pop", "%r11");
  ADD_INSTR("pop", "%r12");
  ADD_INSTR("call", "getcontext"); // Execution will resume here

  // Restore the %rdi and %r10 registers
  ADD_INSTR("movq", "%r11, %rdi");
  ADD_INSTR("movq", "%r12, %r10");

  // Check for an exception
  ADD_INSTR("push", "%r10");
  ADD_INSTR("call", "_check_exception");
  ADD_INSTR("pop", "%r10");
  ADD_INSTR("cmp", "$1, %rax");
  ADD_INSTR("je", catch_label);

  // Generate the statements in the try block
  for (stmt_t s = try_catch->try_block; s; s = s->next) {
    ADD_BLOCK(gen_stmt(ctx, s));
  }
  ADD_INSTR("call", "_clear_try");
  ADD_INSTR("jmp", end_label);

  // Generate the statements in the catch block
  ADD_LABEL(catch_label);
  ADD_INSTR("push", "%r10");
  ADD_INSTR("push", "%rdi");
  ADD_INSTR("push", "%rsi");
  ADD_BLOCK(gen_lval_expr(ctx, try_catch->catch_lval, "%rsi"));
  ADD_INSTR("call", "_get_exception");
  ADD_INSTR("movq", "%rax, %rdi");
  ADD_INSTR("call", "__assign_simple");
  ADD_INSTR("pop", "%rsi");
  ADD_INSTR("pop", "%rdi");
  ADD_INSTR("call", "_clear_try");
  ADD_INSTR("pop", "%r10");
  for (stmt_t s = try_catch->catch_block; s; s = s->next) {
    ADD_BLOCK(gen_stmt(ctx, s));
  }
  
  // Cleanup the try-catch
  ADD_LABEL(end_label);
  RETURN_BUFFER;
}

char* gen_throw_stmt(context_t ctx, throw_stmt_t throw) {
  CREATE_BUFFER;
  ADD_BLOCK(gen_expr(ctx, throw->exception, "%rdi"));
  ADD_INSTR("call", "_throw_exception");
  RETURN_BUFFER;
}

bool has_main(root_t root) {
  for (mod_t mod = root->mods; mod; mod = mod->next)
    if (strcmp("Main", mod->name) == 0)
      return true;
  return false;
}

char* codegen(root_t root, type_node_t *g) {
  graph = g;
  build_file_no_map(root);
  CREATE_BUFFER;

  // Generate text segment
  ADD_BLOCK(gen_text_segment(root));

  // Generate data segment
  char *data_segment = remove_empty_lines(gen_data_segment());

  // data_segment will have 2 \n characters if it is empty
  if (count_char_occurences(data_segment, '\n') > 3) {
    ADD_BLOCK(data_segment);
  }

  RETURN_BUFFER;
}
