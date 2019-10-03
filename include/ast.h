#ifndef _AST_H
#define _AST_H

#include "util.h"

typedef struct root_st          *root_t;
typedef struct mod_st           *mod_t;
typedef struct decl_st          *decl_t;
typedef struct id_list_st       *id_list_t;
typedef struct assign_st        *assign_t;
typedef struct const_decl_st    *const_decl_t;
typedef struct type_decl_st     *type_decl_t;
typedef struct var_decl_st      *var_decl_t;
typedef struct fun_decl_st      *fun_decl_t;
typedef struct const_st         *const_t;
typedef struct type_st          *type_t;
// typedef struct var_st           *var_t;
// typedef struct fun_st           *fun_t;

// Expressions
typedef struct expr_st          *expr_t;
typedef struct literal_st       *literal_t;
typedef struct unary_st         *unary_t;
typedef struct binary_st        *binary_t;
typedef struct ternary_st       *ternary_t;
typedef struct call_st          *call_t;
typedef struct range_st         *range_t;
typedef struct morph_expr_st    *morph_expr_t;
typedef struct morph_st         *morph_t;
typedef struct morph_chain_st   *morph_chain_t;

typedef struct boolean_st       *boolean_t;
typedef struct enum_st          *enum_t;
typedef struct rec_st           *rec_t;
typedef struct arg_st           *arg_t;

//statments
typedef struct stmt_st          *stmt_t;
typedef struct stmt_list_st     *stmt_list_t;
typedef struct assign_stmt_st   *assign_stmt_t;
typedef struct cond_stmt_st     *cond_stmt_t;
typedef struct for_stmt_st      *for_stmt_t;
typedef struct loop_stmt_st     *loop_stmt_t;
typedef struct case_stmt_st     *case_stmt_t;
typedef struct expr_stmt_st     *expr_stmt_t;

// Entry node in the AST
struct root_st {
  mod_t mods;
};


// A declaration
struct mod_st {
  // Module name
  char* name;

  // Inner module declarations
  decl_t decl;

  // Next module in the declaration sequence
  mod_t next; // NULLABLE
};


// A sequence of declarations
struct decl_st {
  const_decl_t constants; // NULLABLE
  type_decl_t types; // NULLABLE
  var_decl_t vars; // NULLABLE
  fun_decl_t funs; // NULLABLE
  mod_t mods; // NULLABLE
};


// A variable declaration
struct var_decl_st {
  // name of the variable
  id_list_t names;

  // type of the variable
  type_t type;

  // The assignment
  assign_t assign; // NULLABLE

  // next variable in the declaration
  var_decl_t next; // NULLABLE
};


// A declaration of a constant
struct const_decl_st {
  // Constant name
  char* name;

  // Constant type
  type_t ty;

  // Expression assigned to the constant
  assign_t assign;

  // next constant in declaration sequence
  const_decl_t next; // NULLABLE
};


// A function declaration
struct fun_decl_st {
  // name of the function
  char* name;

  // Function args
  arg_t args;

  // return type of the function
  type_t ret_type;

  // Inner function declarations
  decl_t decl;

  // next function in the declaration
  fun_decl_t next; // NULLABLE
};


// A type declaration
struct type_decl_st {
  // Name of the type
  char* name;

  // The type expression we are declaring the name to be equal to
  type_t type;

  // Any morphs for this type
  morph_t morphs;

  // The next type in the sequence of declarations
  type_decl_t next; // NULLABLE
};


struct rec_st {
};


// A "type expression", something which evaluates to a concrete type
struct type_st {
  enum {
// Not sure about these, seems like they all fall under the category of
// 'named type'
//    STRING_TY, INTEGER_TY, REAL_TY, BOOLEAN_TY, ARRAY_TY, HASH_TY, LIST_TY,
    NAME_TY, REC_TY, ENUM_TY, MORPH_TY
  } kind;
  union {
    char *name_ty;
    rec_t rec_ty;
    enum_t enum_ty;
    morph_chain_t morph_ty;
  } u;
};


// A specific path to follow for a morph
struct morph_chain_st {
  // Type of at this point in the chian
  type_t ty;

  // Type of path we are coming from
  enum {
    DIRECT_PATH,
    BEST_PATH
  } path; // -> is DIRECT_PATH, ... is BEST_PATH

  // points to next morph in chain
  morph_chain_t next; // NULLABLE
};


// A comma separated list of identifiers
struct id_list_st {
  // name of the id
  char *name;

  // next id in the list
  id_list_t next; // NULLABLE
};


// An assignment
struct assign_st {
  enum assign_kind {
    SIMPLE_AS,
    DEEP_AS,
    PLUS_AS,
    MINUS_AS,
    MULT_AS,
    DIV_AS,
    MOD_AS,
    OR_AS,
    AND_AS,
    XOR_AS
  } kind;

  // Expression to be assigned to
  expr_t expr;
};


// Any full expression (aka rvalue)
struct expr_st {
  // Type of the expression
  type_t type;

  // Tagged union of various kinds of expressions
  enum {
    ID_EX, LITERAL_EX, UNARY_EX, BINARY_EX, TERNARY_EX,
    CALL_EX, RANGE_EX
  } kind;

  union {
    char         *id_ex;
    literal_t     literal_ex;
    unary_t       unary_ex;
    binary_t      binary_ex;
    ternary_t     ternary_ex;
    call_t        call_ex;
    range_t       range_ex;
    morph_expr_t  morph_ex;
  } u;
};

// One operand expression
struct unary_st {
  // The operator
  enum {
    NOT_OP,
    BIT_NOT_OP,
    NEG_OP, // (unimplemented) TODO: how does this parse?
    PRE_INC_OP,
    PRE_DEC_OP,
    POST_INC_OP,
    POST_DEC_OP
  } op;

  // The operand
  expr_t expr;
};


// Two operand expression
struct binary_st {
  // The operator
  enum {
    PLUS_OP,
    MINUS_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,
    AND_OP,
    OR_OP,
    XOR_OP,
    BIT_AND_OP,
    BIT_OR_OP,
    BIT_XOR_OP,
    SHIFT_RIGHT_OP,
    SHIFT_LEFT_OP,
    EQ_EQ_OP,
    LT_OP,
    GT_OP,
    NOT_EQ_OP,
    LT_EQ_OP,
    GT_EQ_OP
  } op;

  // The operands
  expr_t left;
  expr_t right;
};


// Three operand expression
struct ternary_st {
  // The operator
  enum {
    IF_ELSE_OP
  } op;

  // The operands
  expr_t left;
  expr_t middle;
  expr_t right;
};


// A literal expression
struct literal_st {
  // Tagged union of literal possibilities
  enum {
    STRING_LIT, INTEGER_LIT, REAL_LIT,
    BOOLEAN_LIT
  } kind;

  union {
    // These three options seem redundant but it seems best to keep them around
    // for logical consistency
    char *string_lit;
    char *integer_lit;
    char *real_lit;
    enum {
      TRUE_BOOL,
      FALSE_BOOL
    } bool_lit;
  } u;
};


// Explicit morph
struct morph_expr_st {
  expr_t expr;
  morph_chain_t morph;
};

struct morph_st {
};

struct arg_st {
  char*  name;
  type_t type;
  arg_t  next;
};

// statements
struct stmt_st {
  enum{
    COND_STMT, FOR_STMT, LOOP_STMT,
    CASE_STMT, ASSIGN_STMT, EXPR_STMT  
  } kind;

  union{
    cond_stmt_t    cond_stmt;
    case_stmt_t    case_stmt;
    for_stmt_t     for_stmt;
    loop_stmt_t    loop_stmt;
    assign_stmt_t  assign_stmt;
    expr_stmt_t    expr_stmt;
  } u;

  stmt_t next;
};

struct cond_stmt_st {
  expr_t test;
  stmt_t body; // NULLABLE
  stmt_t else_stmt; // NULLABLE
};

struct for_stmt_st {
  char   *iter;
  expr_t  list;
  stmt_t  body;
}; 

struct loop_stmt_st {
  stmt_t body;
};

struct case_stmt_st {
  expr_t test;
  // TODO
};

struct assign_stmt_st {
  expr_t   lval;
  assign_t assign;
};

struct expr_stmt_st {
  expr_t expr;
};


// prototypes
root_t parse(ll_t ll_tokens);

#endif
