#include "stdlib.h"
#include "string.h"

#include "symbol_structs.h"

scope_t scope_new(scope_t parent) {
  scope_t scope = malloc(sizeof(struct scope_st));

  scope->parent = parent;
  scope->symbols = NULL;

  return scope;
}

void add_symbol(scope_t scope, symbol_t symbol) {
  symbol_t curr = scope->symbols;
  for (; curr->next; curr = curr->next);

  curr->next = symbol;
}

symbol_t symbol_new(char *name) {
  symbol_t symbol = malloc(sizeof(struct symbol_st));
  memset(symbol, 0, sizeof(struct symbol_st)); // to be safe

  symbol->name = name;

  return symbol;
}

