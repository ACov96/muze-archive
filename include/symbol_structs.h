#pragma once

typedef struct symbol_st *symbol_t;
typedef struct scope_st  *scope_t;

#include "type.h"

struct symbol_st {
  char *name;
  enum {
    VAR_SYMB,
    TYPE_SYMB,
    FUNC_SYMB,
    MOD_SYMB,
  } kind;
  union {
    struct {
      int mut;
      type_id type;
    } var;

    struct {
      type_id id;
    } type;

    struct {
      type_id ret_type;
      type_id *param_types;
    } func;

    struct {
    } mod;
  } u;
  scope_t scope;
  symbol_t next;
};

struct scope_st {
  scope_t parent;
  symbol_t symbols;
};

