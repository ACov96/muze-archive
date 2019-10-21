#ifndef _CONTEXT_H
#define _CONTEXT_H

#define ADDER_HEADER(T) void ctx_add_ ## T (context_t ctx, char *id)

typedef struct context_st *context_t;
typedef struct static_link_st *static_link_t;

struct static_link_st {
  int offset;
  int levels;
};

context_t ctx_new();
void ctx_set_parent(context_t ctx, context_t parent);
context_t ctx_pop_child(context_t ctx);
static_link_t ctx_get_id(context_t ctx, char *id);
char* ctx_pop_break_label(context_t ctx);
char* ctx_curr_break_label(context_t ctx);

ADDER_HEADER(constant);
ADDER_HEADER(variable);
ADDER_HEADER(argument);
ADDER_HEADER(break_label);

#endif
