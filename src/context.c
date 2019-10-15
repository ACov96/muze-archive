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

#define GET(T) static_link_t ctx_get_ ## T (context_t ctx, char *id, static_link_t sl) { \
    if (sl == NULL) {                                                   \
      sl = malloc(sizeof(struct static_link_st));                       \
      sl->offset = 0;                                                   \
      sl->levels = 0;                                                   \
    }                                                                   \
    for (ll_t l = ctx->T ## s; l; l = l->next)                          \
      if (strcmp((char*)l->val, id) == 0)                               \
        return sl;                                                      \
      else                                                              \
        sl->offset++;                                                   \
    if (ctx->parent != NULL) {                                          \
      sl->offset = 0;                                                   \
      sl->levels = 1;                                                   \
      return ctx_get_ ## T (ctx->parent, id, sl);                       \
    }                                                                   \
    return NULL;                                                        \
  }


struct context_st {
  char *module_name;
  context_t parent;
  ll_t arguments;
  ll_t constants;
  ll_t variables;
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

GET(constant);
GET(argument);
GET(variable);

static_link_t ctx_get_id(context_t ctx, char *id) {
  static_link_t sl = ctx_get_argument(ctx, id, NULL);
  if (sl) return sl;
  sl = ctx_get_constant(ctx, id, NULL);
  if (sl) return sl;
  sl = ctx_get_variable(ctx, id, NULL);
  if (sl) return sl;
  return NULL;
}
