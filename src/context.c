#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "util.h"
#include "ast.h"

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

typedef struct func_map_st *func_map_t;

struct func_map_st {
  char *full_name;
  fun_decl_t f;
};

struct context_st {
  enum {
        FUNC_CTX,
        MOD_CTX,
  } kind;
  char *scope_name;
  context_t parent;
  context_t curr_mod;
  ll_t arguments;
  ll_t constants;
  ll_t variables;
  ll_t break_labels;
  ll_t functions;
};

/* PROTOTYPES */
// None

context_t ctx_new() {
  context_t ctx = malloc(sizeof(struct context_st));
  ctx->kind = FUNC_CTX;
  ctx->arguments = NULL;
  ctx->constants = NULL;
  ctx->variables = NULL;
  ctx->parent = NULL;
  ctx->curr_mod = NULL;
  ctx->scope_name = "";
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

ADDER(constant);
ADDER(argument);
ADDER(variable);
ADDER(break_label);

GET(constant);
GET(argument);
GET(variable);

static_link_t ctx_get_id(context_t ctx, char *id) {
  int offset = 0;
  static_link_t sl = malloc(sizeof(struct static_link_st));
  sl->offset = 0;
  sl->is_mod = ctx_get_kind(ctx);
  offset = ctx_get_argument(ctx, id);
  if (offset != -1) {
    sl->offset = offset;
    return sl;
  }
  offset = ctx_get_constant(ctx, id);
  if (offset != -1) {
    sl->offset = offset + ll_length(ctx->arguments);
    return sl;
  }
  offset = ctx_get_variable(ctx, id);
  if (offset != -1) {
    sl->offset = offset + ll_length(ctx->arguments) + ll_length(ctx->constants);
    return sl;
  }
  if (ctx->parent != NULL) {
    sl->offset = 0;
    sl->next = ctx_get_id(ctx->parent, id);
    return sl;
  }
  return NULL; 
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

char* ctx_get_scope_name(context_t ctx) {
  return ctx->scope_name;
}

void ctx_set_scope_name(context_t ctx, char *name) {
  ctx->scope_name = name;
}

void ctx_add_function(context_t ctx, fun_decl_t f) {
  func_map_t fm = malloc(sizeof(struct func_map_st));
  fm->f = f;
  fm->full_name = concat(ctx_get_scope_name(ctx), concat("_", f->name));
  if (ctx->functions == NULL) {
    ctx->functions = ll_new();
    ctx->functions->val = fm;
  } else {
    ll_append(ctx->functions, fm);
  }
}

func_link_t ctx_get_function(context_t ctx, char *id) {
  int levels = 0;
  for (context_t curr = ctx; curr; curr = curr->parent) {
    for (ll_t l = curr->functions; l; l = l->next) {
      func_map_t fm = (func_map_t) l->val;
      if (strcmp(id, fm->f->name) == 0) {
        func_link_t fl = malloc(sizeof(struct func_link_st));
        fl->levels = levels;
        fl->id = fm->full_name;
        return fl;
      }
    }  
    levels++;
  }
  return NULL;
}

void ctx_set_curr_mod(context_t ctx, context_t mod) {
  ctx->curr_mod = mod;
}

context_t ctx_get_curr_mod(context_t ctx) {
  return ctx->curr_mod;
}

void ctx_set_func(context_t ctx) {
  ctx->kind = FUNC_CTX;
}

void ctx_set_mod(context_t ctx) {
  ctx->kind = MOD_CTX;
}

int ctx_get_kind(context_t ctx) {
  return ctx->kind;
}
