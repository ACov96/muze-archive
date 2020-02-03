#include "stdlib.h"
#include "string.h"

#include "symbol_structs.h"

// Returns NULL on failure to find the symbol
symbol_t find_symbol(scope_t scope, char* name) {
  while (scope) {

    // search the current scope
    for (symbol_t curr = scope->symbols;
         curr;
         curr = curr->next) {
      if (!strcmp(curr->name, name)) {
        return curr;
      }
    }

    // symbol not found in scope, check its parent
    scope = scope->parent;
  }

  // symbol not found
  return NULL;
}

scope_t scope_new(scope_t parent) {
  scope_t scope = malloc(sizeof(struct scope_st));
  memset(scope, 0, sizeof(struct scope_st)); // to be safe

  scope->parent = parent;
  scope->symbols = NULL;

  return scope;
}

int add_symbol(scope_t scope, symbol_t symbol) {
  symbol_t found = find_symbol(scope, symbol->name);

  if (found) {
    return 0; // TODO actually handle this case correctly
  }

  symbol_t *curr = &(scope->symbols);

  for (; *curr; curr = &((*curr)->next));

  *curr = symbol;
  symbol->scope = scope;

  return 1;
}

symbol_t symbol_new(char *name) {
  symbol_t symbol = malloc(sizeof(struct symbol_st));
  memset(symbol, 0, sizeof(struct symbol_st)); // to be safe

  symbol->name = name;

  return symbol;
}

