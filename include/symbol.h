#pragma once

#include "symbol_structs.h"

scope_t scope_new(scope_t parent);
void add_symbol(scope_t scope, symbol_t symbol);

symbol_t symbol_new(char *name);

