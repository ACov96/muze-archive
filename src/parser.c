#include <stdio.h>
#include <stdbool.h>

#include "ast.h"
#include "lexer.h"

enum token current_token;

#define call(fn, ll) \
  if (!fn(ll)) \
    return false; \
  ll = ll->next;

#define match(t, ll) \
  if (beget(ll)->tok != t) \
    return false; \
  ll = ll->next;

#define beget(ll) \
   ((token_t)(ll->val))

bool parse_decl(ll_t tokens) {
  return true;
}

bool parse_module(ll_t tokens) {
  if (LEXEOF) {
  }
  return true;
}

bool parse_module_inner(ll_t tokens) {
  match(MOD, tokens);
  call(parse_decl, tokens);
  match(DOM, tokens);
  //call(parse_module,tokens);
  return true;
}

bool parse(ll_t tokens) {
  parse_module_inner(tokens);
  return true;
}

