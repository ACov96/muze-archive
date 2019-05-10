#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "lexer.h"

#define LL_NAME tokens
#define LL_OUT tok_out

#define MATCH_FUN(fn, res) \
  (res = fn(LL_NAME, &LL_NAME))

#define MATCH_TOK(t) \
  (BEGET->tok == t)

#define EXPECT_FUN(fn, res) \
  if (!MATCH_FUN(fn, res)) \
    return NULL; \
  NEXT;

#define EXPECT_TOK(t) \
  if (!MATCH_TOK(t)) { \
    snprintf(last_mismatch, BUFSIZ, "Expected %s, got %s", \
        token_names[t], token_names[BEGET->tok]); \
    return NULL; \
  } \
  NEXT;

#define PARSE_RETURN(ret) \
  *LL_OUT = LL_NAME; \
  return ret; 

// Gets the current token
#define BEGET \
  ((token_t)(LL_NAME->val))

#define NEXT \
  LL_NAME = LL_NAME->next;

#define PARSE_PARAMS ll_t LL_NAME, ll_t *LL_OUT

// most recent mismatch
static char last_mismatch[BUFSIZ];

// Prototypes
static decl_t parse_decl(PARSE_PARAMS);
static mod_t parse_module_decl(PARSE_PARAMS);
static type_decl_t parse_type_decl(PARSE_PARAMS);
static const_t parse_const_decl(PARSE_PARAMS);
static fun_t parse_fun_decl(PARSE_PARAMS);
static var_t parse_vars_decl(PARSE_PARAMS);
static type_t parse_type_expr(PARSE_PARAMS);
static expr_t parse_expr(PARSE_PARAMS);
static morph_t parse_morph_chain(PARSE_PARAMS);
static char* parse_arithmetic_expr(PARSE_PARAMS);
static literal_t parse_literal(PARSE_PARAMS);
static boolean_t parse_boolean(PARSE_PARAMS);

static expr_t parse_expr(PARSE_PARAMS) {
  expr_t ex = malloc(sizeof(struct expr_st));

  printf("inside parse_expr()\n");
  
  if (MATCH_TOK(STRING_VAL) 
     || MATCH_TOK(INT_VAL) 
     || MATCH_TOK(REAL_VAL)   
     || MATCH_TOK(IDENTIFIER)) {
    if (MATCH_FUN(parse_math_expr, ex->u.math)){
       ex->kind = MATHEMATICAL_EX;
       NEXT;
    }
    else if(MATCH_TOK(IDENTIFIER)){
      ex->kind = ID_EX;
      ex->u.id = BEGET->val;
      NEXT;
    }
    else if (MATCH_FUN(parse_literal, ex->u.literal)){
       ex->kind = LITERAL_EX;
       NEXT;
    }
  }
  else if (MATCH_TOK(TRUE) || MATCH_TOK(FALSE)){
    ex->kind = BOOLEAN;
    EXPECT_FUN(parse_boolean, ex->u.boolean)
  }
  PARSE_RETURN(ex);
 }

static boolean_t parse_boolean(PARSE_PARAMS) {
  boolean_t bool = malloc(sizeof(struct boolean_st));

  printf("inside parse_boolean\n");
  
  if (MATCH_TOK(TRUE))
    bool->val = TRUE_BOOL;
  else if (MATCH_TOK(FALSE))
    bool->val = FALSE_BOOL;
  
  PARSE_RETURN(bool);
}

static literal_t parse_literal(PARSE_PARAMS) {
  literal_t lit = malloc(sizeof(struct literal_st));

  printf("inside parse_literal()\n");
  
  if (MATCH_TOK(STRING_VAL)){
    lit->kind = STRING_LIT;
    lit->u.str = BEGET->val;
  }
  else if (MATCH_TOK(INT_VAL)) {
    lit->kind = INTEGER_LIT;
    lit->u.integer = atoi(BEGET->val);
  }
  else if (MATCH_TOK(REAL_VAL)) {
    lit->kind = REAL_LIT;
    lit->u.real = atof(BEGET->val);
  }
  
  PARSE_RETURN(lit);
}

static char* parse_arithmetic_expr(PARSE_PARAMS) {
  char* ex;

  return NULL;
}

static type_t parse_type_expr(PARSE_PARAMS) {
  type_t ty = malloc(sizeof(struct type_st));

  printf("inside parse_type_expr()\n");

  if (MATCH_TOK(STRING))
   ty->kind = TY_STRING;
  else if (MATCH_TOK(INTEGER))
    ty->kind = TY_INTEGER;
  else if (MATCH_TOK(REAL))
    ty->kind = TY_REAL;
  else if (MATCH_TOK(BOOLEAN))
    ty->kind = TY_BOOLEAN;
  else if (MATCH_TOK(ARRAY))
    ty->kind = TY_ARRAY;
  else if (MATCH_TOK(REC))
    ty->kind = TY_REC;
  else if (MATCH_TOK(HASH))
    ty->kind = TY_HASH;
  else if (MATCH_TOK(LIST))
    ty->kind = TY_LIST;
  else if (MATCH_TOK(IDENTIFIER))
    ty->kind = TY_NAME;

  PARSE_RETURN(ty);

}

static morph_t parse_morph_chain(PARSE_PARAMS) {
  morph_t morph = malloc(sizeof(struct morph_st));

  printf("inside parse_morph_chain()\n");
  
  EXPECT_FUN(parse_type_expr, morph->ty);
  if(MATCH_TOK(DOT_DOT_DOT)){
    NEXT;
    MATCH_FUN(parse_morph_chain, morph->next);
  }
  return morph;
}

static fun_t parse_fun_decl(PARSE_PARAMS) {
  return NULL;
}

static var_t parse_vars_decl(PARSE_PARAMS) {
  return NULL;
}

// parse type declarations
static type_decl_t parse_type_decl(PARSE_PARAMS) {
  type_decl_t ty = malloc(sizeof(struct type_decl_st));

  printf("inside parse_type_decl()\n");

  ty->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(EQ);
  EXPECT_FUN(parse_type_expr, ty->type);
  // check if type dec has a morph chain
  if (MATCH_TOK(DOT_DOT_DOT)){
    NEXT;
    EXPECT_FUN(parse_morph_chain, ty->morphs);
  }
  EXPECT_TOK(SEMICOLON);
  if (MATCH_TOK(MU)){
    //some morph stuff
    printf("inside MU block\n");
    EXPECT_TOK(UM);
  }

  if (MATCH_TOK(IDENTIFIER))
    EXPECT_FUN(parse_type_decl,ty->next);
  
  PARSE_RETURN(ty);
}

// parse constant declarations
static const_t parse_const_decl(PARSE_PARAMS) {
  const_t con;
  con = malloc(sizeof(struct const_st));

  printf("inside parse_const_decl()\n");
  con->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type_expr, con->ty);
  EXPECT_TOK(EQ);
  EXPECT_FUN(parse_expr, con->expr);
  EXPECT_TOK(SEMICOLON);

  if (MATCH_TOK(IDENTIFIER))
    EXPECT_FUN(parse_const_decl,con->next);
  
  PARSE_RETURN(con);
}

// parse declarations
static decl_t parse_decl(PARSE_PARAMS) {
  decl_t decl;
  decl = malloc(sizeof(struct decl_st));

  printf("inside parse_decl()\n");
  if (MATCH_TOK(CONST)){
    NEXT;
    EXPECT_FUN(parse_const_decl, decl->consts);
  }
  if (MATCH_TOK(TYPE)){
    NEXT;
    MATCH_FUN(parse_type_decl, decl->types);
  }
  if (MATCH_TOK(VAR)){
    NEXT;
    MATCH_FUN(parse_vars_decl, decl->vars);
  }
  if (MATCH_TOK(FUN)){
    NEXT;
    MATCH_FUN(parse_fun_decl, decl->funs);\
  }
  if (MATCH_TOK(MOD)){
    NEXT;
    MATCH_FUN(parse_module_decl, decl->mods);
  }

  PARSE_RETURN(decl);
}

// parse module decalarations
static mod_t parse_module_decl(PARSE_PARAMS) {
  mod_t mod;
  mod = malloc(sizeof(mod_t));

  printf("begining mod parse\n");
  EXPECT_TOK(MOD);
  EXPECT_TOK(IDENTIFIER);
  EXPECT_FUN(parse_decl, mod->decl);
  EXPECT_TOK(DOM);

  MATCH_FUN(parse_module_decl, mod->next);

  PARSE_RETURN(mod);
}

// start parse
root_t parse(ll_t tokens) {
  root_t root;
  root = malloc(sizeof(root));

  printf("inside parse()\n");
  EXPECT_FUN(parse_module_decl, root->mods);

  printf("done parsing\n");
  return root;
}

