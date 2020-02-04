#pragma once

#include "symbol_structs.h"

symbol_t find_symbol(scope_t scope, char* name);

scope_t scope_new(scope_t parent);
int add_symbol(scope_t scope, symbol_t symbol);

symbol_t symbol_new(char *name);

