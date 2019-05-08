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
static type_t parse_type_decl(PARSE_PARAMS);
static const_t parse_const_decl(PARSE_PARAMS);
static fun_t parse_fun_decl(PARSE_PARAMS);
static var_t parse_vars_decl(PARSE_PARAMS);
static expr_t parse_expr(PARSE_PARAMS);

static type_t parse_type_decl(PARSE_PARAMS) {
  type_decl_t type;
  type = malloc(sizeof(struct type_decl_st));

  printf("inside parse_type_decl()\n");

  EXPECT_TOK(TYPE);
  type->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  type_t t = malloc(sizeof(struct type_st));
  t->kind = BEGET->val;
  
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(SEMICOLON);
  if (MATCH_TOK(MU)){
    //some morph stuff
    printf("inside MU block\n");
    EXPECT_TOK(UM);
  }
  
  PARSE_RETURN(type);
}

static const_t parse_const_decl(PARSE_PARAMS) {
  const_t constant;
  constant = malloc(sizeof(struct const_st));

  printf("inside parse_const_decl()\n");
  EXPECT_TOK(CONST);
  constant->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(COLON);
  //MATCH_FUN(, ); need to handle type expressions somehow
  EXPECT_TOK(EQ);
  //MATCH_FUN(parse_expr, constant->expr)
  EXPECT_TOK(SEMICOLON);
  
  return NULL;
}

static fun_t parse_fun_decl(PARSE_PARAMS) {
  return NULL;
}

static var_t parse_vars_decl(PARSE_PARAMS) {
  return NULL;
}

 static expr_t parse_expr(PARSE_PARAMS) {
   /*
   expr_t ex;
   ex = malloc(sizeof(struct expr_t));
   
   if (MATCH_TOK(STRING)){
     
   }else if (MATCH_TOK(INTEGER)){
     
   }else if (MATCH_TOK(REAL)){
     
   }else if (MATCH_TOK(BOOLEAN)){
     
   }else if (MATCH_TOK()){
     
   }
   */
   return NULL;
 }

static decl_t parse_decl(PARSE_PARAMS) {
  decl_t decl;
  decl = malloc(sizeof(struct decl_st));

  printf("inside parse_decl()\n");
  MATCH_FUN(parse_const_decl, decl->consts);
  MATCH_FUN(parse_type_decl, decl->types);
  MATCH_FUN(parse_vars_decl, decl->vars);
  MATCH_FUN(parse_fun_decl, decl->funs);
  MATCH_FUN(parse_module_decl, decl->mods);

  PARSE_RETURN(decl);
}

static mod_t parse_module_decl(PARSE_PARAMS) {
  mod_t mod;
  mod = malloc(sizeof(mod_t));

  printf("begining mod parse\n");
  EXPECT_TOK(MOD);
  printf("parsed MOD token\n");
  EXPECT_FUN(parse_decl, mod->decl);
  EXPECT_TOK(DOM);

  MATCH_FUN(parse_module_decl, mod->next);

  PARSE_RETURN(mod);
}

root_t parse(ll_t tokens) {
  root_t root;
  root = malloc(sizeof(root));

  printf("inside parse()\n");
  EXPECT_FUN(parse_module_decl, root->mods);

  printf("done parsing\n");
  return root;
}

