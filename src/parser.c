#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "lexer.h"

#define LL_NAME tokens
#define LL_OUT tok_out

#define MATCH_FUN(fn, res) \
  (res == fn(LL_NAME, &LL_NAME))

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
static morph_chain_t parse_morph_chain(PARSE_PARAMS);
static char* parse_arithmetic_expr(PARSE_PARAMS);
static literal_t parse_literal(PARSE_PARAMS);
static boolean_t parse_boolean(PARSE_PARAMS);

static expr_t parse_expr(PARSE_PARAMS) {
  expr_t ex = malloc(sizeof(expr_t));
        
  if (MATCH_TOK(IDENTIFIER)){
    ex->kind = ID_EX;
    ex->u.id = BEGET->val;
  }
  else if (MATCH_FUN(parse_literal, ex->u.literal))
    ex->kind = LITERAL_EX;

  PARSE_RETURN(ex);
 }

static boolean_t parse_boolean(PARSE_PARAMS) {
  boolean_t bool = malloc(sizeof(boolean_t));

  if (MATCH_TOK(TRUE))
    bool->val = TRUE_BOOL;
  else if (MATCH_TOK(FALSE))
    bool->val = FALSE_BOOL;
  
  PARSE_RETURN(bool);
}

static literal_t parse_literal(PARSE_PARAMS) {
  literal_t lit = malloc(sizeof(literal_t));
  
  if (MATCH_TOK(STRING_VAL)){
    lit->kind = STRING_LIT;
    lit->val = BEGET->val;
  }
  else if (MATCH_TOK(INT_VAL)) {
    lit->kind = INTEGER_LIT;
    lit->val = BEGET->val;
  }
  else if (MATCH_TOK(REAL_VAL)) {
    lit->kind = REAL_LIT;
    lit->val = BEGET->val;
  }
  else if (MATCH_TOK(TRUE)){
    lit->kind = BOOLEAN_LIT;
    lit->val = "true";
  }
  else if (MATCH_TOK(FALSE)){
    lit->kind = BOOLEAN_LIT;
    lit->val = "false";
  }
  PARSE_RETURN(lit);
    
}

static char* parse_arithmetic_expr(PARSE_PARAMS) {
  return NULL;
}

static type_t parse_type_expr(PARSE_PARAMS) {
  type_t ty = malloc(sizeof(type_t));

  if (MATCH_TOK(STRING))
   ty->kind = STRING_TY;
  else if (MATCH_TOK(INTEGER))
    ty->kind = INTEGER_TY;
  else if (MATCH_TOK(REAL))
    ty->kind = REAL_TY;
  else if (MATCH_TOK(BOOLEAN))
    ty->kind = BOOLEAN_TY;
  else if (MATCH_TOK(ARRAY))
    ty->kind = ARRAY_TY;
  else if (MATCH_TOK(REC))
    ty->kind = REC_TY;
  else if (MATCH_TOK(HASH))
    ty->kind = HASH_TY;
  else if (MATCH_TOK(LIST))
    ty->kind = LIST_TY;
  else if (MATCH_TOK(IDENTIFIER))
    ty->kind = NAME_TY;

  PARSE_RETURN(ty);

}

static morph_chain_t parse_morph_chain(PARSE_PARAMS) {
  morph_chain_t morph = malloc(sizeof(morph_t));

  if (MATCH_TOK(DOT_DOT_DOT)){
    morph->path = DIRECT_PATH;
    NEXT;
    EXPECT_FUN(parse_type_expr, morph->ty);
  }
  else if (MATCH_TOK(ARROW)){
    morph->path = BEST_PATH;
    NEXT;
    EXPECT_FUN(parse_type_expr, morph->ty);
  }
  else
    EXPECT_TOK(DOT_DOT_DOT);
  
  MATCH_FUN(parse_morph_chain, morph->next);
  PARSE_RETURN(morph);
}

static fun_t parse_fun_decl(PARSE_PARAMS) {
  return NULL;
}

static var_t parse_vars_decl(PARSE_PARAMS) {
  return NULL;
}

// parse type declarations
static type_decl_t parse_type_decl(PARSE_PARAMS) {
  type_decl_t ty = malloc(sizeof(type_decl_t));
  
  ty->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(EQ);
  MATCH_FUN(parse_morph_chain, ty->morphs);
  EXPECT_FUN(parse_type_expr, ty->type);
  EXPECT_TOK(SEMICOLON);
  if (MATCH_TOK(MU)){
    NEXT;
    //some morph stuff
    EXPECT_TOK(UM);
  } 
  MATCH_FUN(parse_type_decl,ty->next);
  PARSE_RETURN(ty);
}

// parse constant declarations
static const_t parse_const_decl(PARSE_PARAMS) {
  const_t con;
  con = malloc(sizeof(const_t));

  con->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type_expr, con->ty);
  EXPECT_TOK(EQ);
  EXPECT_FUN(parse_expr, con->expr);
  EXPECT_TOK(SEMICOLON);
  MATCH_FUN(parse_const_decl, con->next);
  PARSE_RETURN(con);
}

// parse declarations block
static decl_t parse_decl(PARSE_PARAMS) {
  decl_t decl = malloc(sizeof(decl_t));
  
  if (MATCH_TOK(CONST)){
    NEXT;
    MATCH_FUN(parse_const_decl, decl->constants);
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
  PARSE_RETURN(decl);
}

// parse module decalarations
static mod_t parse_module_decl(PARSE_PARAMS) {
  mod_t mod;
  mod = malloc(sizeof(mod_t));

  EXPECT_TOK(MOD);
  mod->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  MATCH_FUN(parse_decl, mod->decl);
  EXPECT_TOK(DOM);
  EXPECT_TOK(IDENTIFIER);
  MATCH_FUN(parse_module_decl, mod->next);
  PARSE_RETURN(mod);
}

// start parse
root_t parse(ll_t tokens) {
  root_t root;
  root = malloc(sizeof(root_t));

  EXPECT_FUN(parse_module_decl, root->mods);
  printf("done parsing\n");
  return root;
}

