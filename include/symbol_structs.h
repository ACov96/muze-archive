#pragma once

typedef struct symbol_st *symbol_t;
typedef struct scope_st  *scope_t;

struct symbol_st {
  char *name;
  enum {
    VAR_SYMB,
    CONST_SYMB,
    ARG_SYMB,
    TYPE_SYMB,
    FUNC_SYMB,
    MOD_SYMB,
  } kind;
  union {
  } u;
  scope_t scope;
  symbol_t next;
};

struct scope_st {
  scope_t parent;
  symbol_t symbols;
};

