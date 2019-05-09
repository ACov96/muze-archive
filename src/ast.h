#ifndef _AST_H
#define _AST_H

#include "util.h"

typedef struct root_st *root_t;
typedef struct mod_st *mod_t;
typedef struct decl_st *decl_t;
typedef struct var_st *var_t;
typedef struct type_decl_st *type_decl_t;
typedef struct const_st *const_t;
typedef struct var_st *var_t;
typedef struct fun_st *fun_t;
typedef struct expr_st * expr_t;
typedef struct ident_st *ident_t;
typedef struct literal_st *literal_t;
typedef struct unary_st *unary_t;
typedef struct binary_st *binary_t;
typedef struct ternary_st *ternary_t;
typedef struct call_st *call_t;
typedef struct range_st * range_t;
typedef struct morph_expr_st *morph_expr_t;
typedef struct type_st *type_t;
typedef struct morph_st *morph_t;
typedef struct boolean_st *boolean_t;

struct root_st {
  mod_t mods;
};

struct mod_st {
  char* name;
  decl_t decl;
  mod_t next;
};

// 'a' denotes null terminated type array
struct decl_st {
  type_decl_t types;
  const_t consts;
  fun_t funs;
  var_t vars;
  mod_t mods;
};

struct type_decl_st {
  char* name;
  type_t type;
  morph_t morphs;
  type_decl_t next; // points to next type declarations in the block
};

struct type_st {
  enum{
    TY_STRING, TY_INTEGER, TY_REAL, TY_BOOLEAN,
    TY_ARRAY, TY_REC, TY_HASH, TY_LIST, TY_NAME
  } kind;
  
  union {
    char* name; // used for user defined types (TY_NAME)
  };
};

struct morph_st {
  type_t ty;
  morph_t next; // points to next morph in chain
};

struct const_st {
  char* name;
  type_t ty;
  expr_t expr;
  const_t next; // points to next constant declaration in block
};

struct var_st {
  char* name;
  type_t type;
};

struct fun_st {
  char* name;
};

struct expr_st {
  //type_t type; // boolean, integer, string, real, list/range, record

  enum {
    ID_EX, LITERAL_EX, UNARY_EX, BINARY_EX, TERNARY_EX,
    CALL_EX, RANGE_EX, BOOLEAN_EX, MATHEMATICAL_EX
  } kind;

  union {
    char* id;
    literal_t literal;
    unary_t unary;
    binary_t binary;
    ternary_t ternary;
    call_t call;
    range_t range;
    char* math;
    boolean_t boolean;
    morph_expr_t morph_expr;
  } u;
};

struct unary_st {
  expr_t expr;
};

struct binary_st {
  expr_t left;
  expr_t right;
};

struct ternary_st {
  expr_t left;
  expr_t middle;
  expr_t right;
};

struct literal_st {
  enum {
    STRING_LIT, INTEGER_LIT,REAL_LIT,
    BOOLEAN_LIT
  } kind;
  union {
    char* str;
    int integer;
    double real;
    boolean_t boolean;
  } u;
};

struct morph_expr_st {
  // something wild
};

struct boolean_st {
  enum {
    TRUE_BOOL, FALSE_BOOL
  } val;
};
  
// prototypes
root_t parse(ll_t ll_tokens);

#endif
