#ifndef _AST_H
#define _AST_H

#include "util.h"

typedef struct root_st *root_t;
typedef struct mod_st *mod_t;
typedef struct decl_st *decl_t;
typedef struct const_decl_st *const_decl_t;
typedef struct type_decl_st *type_decl_t;
typedef struct var_decl_st *var_decl_t;
typedef struct fun_decl_st *fun_decl_t;
typedef struct const_st *const_t;
typedef struct type_st *type_t;
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
typedef struct morph_st *morph_t;
typedef struct morph_chain_st *morph_chain_t;
typedef struct boolean_st *boolean_t;
typedef struct enum_st *enum_t;
typedef struct rec_st *rec_t;

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
  const_t constants;
  type_decl_t types;
  var_t vars;
  fun_t funs;
  mod_t mods;
};

struct const_st {
  char* name;
  type_t ty;
  expr_t expr;
  const_t next;
};

struct type_decl_st {
  char* name;
  type_t type;
  //morph_chain_t morphs;
  type_decl_t next;
};

struct type_st {
  enum{
    STRING_TY, INTEGER_TY, REAL_TY, BOOLEAN_TY,
    ARRAY_TY, REC_TY, HASH_TY, LIST_TY, NAME_TY,
    MORPH_TY, ENUM_TY
  } kind;
  union {
    rec_t rec_ty;
    enum_t enum_ty;
    morph_chain_t morph_ty;
    char *name_ty;
  } u;
};

struct morph_chain_st {
  type_t ty;
  enum{DIRECT_PATH, BEST_PATH} path; // -> is DIRECT_PATH, ... is BEST_PATH 
  morph_chain_t next; // points to next morph in chain
};


struct var_st {
  char* name;
  type_t type;
  expr_t expr;
  var_t next;
};

struct fun_st {
  char* name;
  fun_t next;
};

struct expr_st {
  type_t type;
  enum {
    ID_EX, LITERAL_EX, UNARY_EX, BINARY_EX, TERNARY_EX,
    CALL_EX, RANGE_EX
  } kind;

  union {
    char* id;
    literal_t literal;
    unary_t unary;
    binary_t binary;
    ternary_t ternary;
    call_t call;
    range_t range;
    morph_expr_t morph_expr; // a = (morph ...integer 2)
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
    STRING_LIT, INTEGER_LIT, REAL_LIT,
    BOOLEAN_LIT
  } kind;
  char* val;
};

struct morph_expr_st {
  expr_t expr;
  morph_chain_t morph;
};

struct boolean_st {
  enum {
    TRUE_BOOL, FALSE_BOOL
  } val;
};

struct morph_st {
};
  
// prototypes
root_t parse(ll_t ll_tokens);

#endif
