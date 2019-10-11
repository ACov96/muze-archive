// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"

/* MACROS */
#define WORD 8
#define CREATE_BUFFER char *buf = ""
#define RETURN_BUFFER return buf
#define PRINT_BUFFER printf("%s\n", buf)

#define ADD_BLOCK(B) buf = concat(buf, B);      \
  buf = concat(buf, "\n")

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

/* PROTOTYPES */
// Helper functions
unsigned int count_consts_and_vars(decl_t decl);
int get_id_offset(char *id, decl_t decl);
char* register_or_get_string_label(char *str);

// Generator functions
char* gen_label(char *label);
char* gen_data_segment();
char* gen_mod(mod_t mod);
char* gen_fun(fun_decl_t fun);
char* gen_stmt(stmt_t stmt);
char* gen_expr_stmt(expr_stmt_t expr);
char* gen_expr(expr_t expr, reg_t out);
char* gen_call_expr(call_t call, reg_t out);
char* gen_literal_expr(literal_t literal, reg_t out);

/* HELPERS */
unsigned int count_consts_and_vars(decl_t decl) {
  unsigned int total = 0;
  for (const_decl_t c = decl->constants; c; c = c->next)
    total++;
  for (var_decl_t var = decl->vars; var; var = var->next)
    for (id_list_t id = id; id; id = id->next)
      total++;
  return total * WORD;
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
    buf = concat(buf, gen_fun(f));
  }
  RETURN_BUFFER;
}

char* gen_fun(fun_decl_t fun) {
  unsigned int preamble_space = count_consts_and_vars(fun->decl);
  char *sub_inst = malloc(32);
  sprintf(sub_inst, "$%d, %%rsp", preamble_space);
  CREATE_BUFFER;
  ADD_LABEL(fun->name);
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", sub_inst);
  for (stmt_t s = fun->stmts; s; s = s->next) {
    ADD_BLOCK(gen_stmt(s));
  }
  RETURN_BUFFER;
}

char* gen_stmt(stmt_t stmt) {
  CREATE_BUFFER;
  switch(stmt->kind) {
  case EXPR_STMT:
    ADD_BLOCK(gen_expr_stmt(stmt->u.expr_stmt));
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
    // TODO
    break;
  default:
    // TODO
    break;
  }
  RETURN_BUFFER;
}

char* gen_expr_stmt(expr_stmt_t expr) {
  return gen_expr(expr->expr, "%rax");
}

char* gen_expr(expr_t expr, reg_t out) {
  CREATE_BUFFER;
  switch(expr->kind) {
  case ID_EX:
    // TODO
    break;
  case LITERAL_EX:
    // TODO
    ADD_BLOCK(gen_literal_expr(expr->u.literal_ex, out));
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
    ADD_BLOCK(gen_call_expr(expr->u.call_ex, out));
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

char* gen_call_expr(call_t call, reg_t out) {
  int arg_idx = 0;
  expr_list_t curr_arg = call->args;
  CREATE_BUFFER;
  PUSH_CALLER_SAVES;

  // Put arguments in registers
  while (curr_arg) {
    ADD_INSTR("push", arg_registers[arg_idx]);
    ADD_BLOCK(gen_expr(curr_arg->expr, arg_registers[arg_idx]));
    curr_arg = curr_arg->next;
    arg_idx++;
  }

  // Make call to function
  ADD_INSTR("call", call->id);

  // Restore argument registers
  for (int i = arg_idx - 1; i >= 0; i--) {
    ADD_INSTR("pop", arg_registers[i]);
  }

  POP_CALLER_SAVES;
  ADD_INSTR("mov", concat("%rax", concat(", ", out)));
  RETURN_BUFFER;
}

char* gen_literal_expr(literal_t literal, reg_t out) {
  char *str_label;
  CREATE_BUFFER;
  switch(literal->kind) {
  case STRING_LIT:
    // TODO
    str_label = concat("$", register_or_get_string_label(literal->u.string_lit));
    str_label = concat(str_label, ", ");
    printf("STRING LABEL: %s\n", str_label);
    ADD_INSTR("mov", concat(str_label, out));
    break;
  case INTEGER_LIT:
    // TODO
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
  CREATE_BUFFER;
  for (ll_t l = strings; l; l = l->next) {
    string_label_t str_label = l->val;
    ADD_LABEL(str_label->label);
    ADD_INSTR(".asciz", concat("\"", concat(str_label->str, "\"")));
  }
  RETURN_BUFFER;
}

char* codegen(root_t root) {
  CREATE_BUFFER;
  ADD_BLOCK("\t.global main\n");
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    ADD_BLOCK(gen_mod(mod));
  }
  ADD_BLOCK("\t.data\n");
  ADD_BLOCK(gen_data_segment());
  RETURN_BUFFER;
}
