#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "util.h"

#define ADDER(T) void ctx_add_ ## T (context_t ctx, char *id) { \
    if (ctx->T ## s == NULL) {                                  \
      ctx->T ## s = ll_new();                                   \
      ctx->T ## s->val = id;                                    \
    }                                                           \
    else {                                                      \
      ll_append(ctx->T ## s, id);                               \
    }                                                           \
  }

#define GET(T) int ctx_get_ ## T (context_t ctx, char *id) {            \
    int offset = 0;                                                     \
    for (ll_t l = ctx->T ## s; l; l = l->next)                          \
      if (strcmp((char*)l->val, id) == 0)                               \
        return offset;                                                  \
      else                                                              \
        offset++;                                                       \
    return -1;                                                          \
  }


struct context_st {
  char *module_name;
  context_t parent;
  ll_t arguments;
  ll_t constants;
  ll_t variables;
  ll_t break_labels;
};

/* PROTOTYPES */
// None

context_t ctx_new() {
  context_t ctx = malloc(sizeof(struct context_st));
  ctx->arguments = NULL;
  ctx->constants = NULL;
  ctx->variables = NULL;
  ctx->parent = NULL;
  ctx->module_name = "";
  return ctx;
}

void ctx_set_parent(context_t ctx, context_t parent) {
  ctx->parent = parent;
}

context_t ctx_pop_child(context_t ctx) {
  context_t parent = ctx->parent;
  // TODO: This is not a sufficient free, sue me. I'll fix it later. - Alex
  free(ctx); 
  return parent;
}

int ctx_get_idx_of_id(context_t ctx, char *id) {
  int check = 0;
  int idx = 0;

  int _list_check(ll_t l) {
    if (l == NULL)
      return -1;
    for(; l; l = l->next)
      if (strcmp(l->val, id) == 0)
        return idx;
      else
        idx++;
    return -1;
  }

  if (ctx->arguments == NULL && ctx->constants == NULL && ctx->variables == NULL)
    return -1;
  check = _list_check(ctx->arguments);
  if (check != -1) return check;
  check = _list_check(ctx->constants);
  if (check != -1) return check;
  check = _list_check(ctx->variables);
  if (check != -1) return check;
  return -1;
}

ADDER(constant);
ADDER(argument);
ADDER(variable);
ADDER(break_label);

GET(constant);
GET(argument);
GET(variable);

static_link_t _ctx_get_id(context_t ctx, char *id, static_link_t sl) {
  int offset;
  if (sl == NULL) {
    sl = malloc(sizeof(struct static_link_st));
    sl->offset = 0;
    sl->levels = 0;
  }
  offset = ctx_get_argument(ctx, id);
  if (offset != -1) {
    sl->offset = offset;
    return sl;
  }
  offset = ctx_get_constant(ctx, id);
  if (offset != -1) {
    sl->offset = offset;
    return sl;
  }
  offset = ctx_get_variable(ctx, id);
  if (offset != -1) {
    sl->offset = offset;
    return sl;
  }
  if (ctx->parent != NULL) {
    sl->offset = 0;
    sl->levels += 1;
    return _ctx_get_id(ctx->parent, id, sl);
  }
  return NULL;
}

static_link_t ctx_get_id(context_t ctx, char *id) {
  return _ctx_get_id(ctx, id, NULL);
}

char* ctx_pop_break_label(context_t ctx) {
  // TODO: There is a memory leak here
  ll_t l = NULL;
  char *label = NULL;
  if (ctx->break_labels == NULL) return NULL;
  if (ctx->break_labels->next == NULL) {
    // There is only one label
    label = ctx->break_labels->val;
    ctx->break_labels = NULL;
    return label;
  }
  for (l = ctx->break_labels; l->next->next; l = l->next);
  label = l->next->val;
  l->next = NULL;
  return label;
} 

char* ctx_curr_break_label(context_t ctx) {
  ll_t l = NULL;
  if (ctx->break_labels == NULL) return NULL;
  for (l = ctx->break_labels; l->next; l = l->next);
  return l->val;
}
