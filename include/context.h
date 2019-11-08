#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "ast.h"

#define ADDER_HEADER(T) void ctx_add_ ## T (context_t ctx, char *id)

typedef struct context_st *context_t;
typedef struct static_link_st *static_link_t;
typedef struct func_link_st *func_link_t;

struct static_link_st {
  int is_mod;
  int offset;
  int levels;
  static_link_t next;
};

struct func_link_st {
  int levels;
  char *id;
};

context_t ctx_new();
void ctx_set_parent(context_t ctx, context_t parent);
context_t ctx_pop_child(context_t ctx);
static_link_t ctx_get_id(context_t ctx, char *id);
char* ctx_pop_break_label(context_t ctx);
char* ctx_curr_break_label(context_t ctx);
char* ctx_get_scope_name(context_t ctx);
void ctx_set_scope_name(context_t ctx, char *name);
func_link_t ctx_get_function(context_t ctx, char *id);
void ctx_add_function(context_t ctx, fun_decl_t f);
void ctx_set_curr_mod(context_t ctx, context_t mod);
context_t ctx_get_curr_mod(context_t ctx);
void ctx_set_func(context_t ctx);
void ctx_set_mod(context_t ctx);
int ctx_get_kind(context_t ctx);

ADDER_HEADER(constant);
ADDER_HEADER(variable);
ADDER_HEADER(argument);
ADDER_HEADER(break_label);

#endif
