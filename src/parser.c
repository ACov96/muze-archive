#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "lexer.h"

#define MATCH_FUN(fn, res, ll) \
  (res = fn(ll))

#define MATCH_TOK(t, ll) \
  (BEGET(ll)->tok == t)

#define EXPECT_FUN(fn, res, ll) \
  if (!MATCH_FUN(fn, res, ll)) \
    return NULL; \
  NEXT(ll);

#define EXPECT_TOK(t, ll) \
  if (!MATCH_TOK(t, ll)) { \
    snprintf(last_mismatch, BUFSIZ, "Expected %s, got %s", \
        token_names[t], token_names[BEGET(ll)->tok]); \
    return NULL; \
  } \
  NEXT(ll);

// Gets the current token
#define BEGET(ll) \
  ((token_t)(ll->val))

#define NEXT(ll) \
  ll = ll->next;

// most recent mismatch
static char last_mismatch[BUFSIZ];

// Prototypes
static decl_t parse_decl(ll_t tokens);
static mod_t parse_module_decl(ll_t tokens);

static decl_t parse_decl(ll_t tokens) {
  decl_t decl;
  decl = malloc(sizeof(decl_t));

  MATCH_FUN(parse_module_decl, decl->mods, tokens);

  return decl;
}

static mod_t parse_module_decl(ll_t tokens) {
  mod_t mod;
  mod = malloc(sizeof(mod_t));

  EXPECT_TOK(MOD, tokens);
  EXPECT_FUN(parse_decl, mod->decl, tokens);
  EXPECT_TOK(DOM, tokens);

  MATCH_FUN(parse_module_decl, mod->next, tokens);

  return mod;
}

root_t parse(ll_t tokens) {
  root_t root;
  root = malloc(sizeof(root));

  EXPECT_FUN(parse_module_decl, root->mods, tokens);

  return root;
}

