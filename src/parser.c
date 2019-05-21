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
  NEXT

#define EXPECT_TOK(t) \
  if (!MATCH_TOK(t)) { \
    PARSE_FAIL("Expected %s, got %s", token_names[t], \
               token_names[BEGET->tok]); \
    return NULL; \
  } \
  NEXT

#define PARSE_RETURN(ret) \
  *LL_OUT = LL_NAME; \
  return ret

#define PARSE_FAIL(...) \
  snprintf(parse_err, BUFSIZ, __VA_ARGS__)

// Gets the current token
#define BEGET \
  ((token_t)(LL_NAME->val))

#define NEXT \
  LL_NAME = LL_NAME->next

#define PARSE_PARAMS ll_t LL_NAME, ll_t *LL_OUT

// most recent mismatch
static char parse_err[BUFSIZ];

// Prototypes
static decl_t parse_decl(PARSE_PARAMS);
static mod_t parse_module_decl(PARSE_PARAMS);
static type_decl_t parse_type_decl(PARSE_PARAMS);
static const_t parse_const_decl(PARSE_PARAMS);
static fun_t parse_fun_decl(PARSE_PARAMS);
static var_t parse_vars_decl(PARSE_PARAMS);
static type_t parse_type(PARSE_PARAMS);
static expr_t parse_expr(PARSE_PARAMS);
static morph_chain_t parse_morph_chain(PARSE_PARAMS);
static char* parse_arithmetic_expr(PARSE_PARAMS);
static literal_t parse_literal(PARSE_PARAMS);
static boolean_t parse_boolean(PARSE_PARAMS);
static type_t parse_type_expr(PARSE_PARAMS);
static arg_t parse_arg_list(PARSE_PARAMS);

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
static arg_t parse_arg_list(PARSE_PARAMS) {
  arg_t arg = malloc(sizeof(arg_t));

  printf("entered parse_arg_list\n");
  printf("%s\n", BEGET->val);

  arg->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  if (MATCH_TOK(COMMA)) {
    NEXT;
    EXPECT_FUN(parse_arg_list, arg->next);
  }
  EXPECT_TOK(COLON);
  printf("parsed COLON\n");
  printf("%s\n", BEGET->val);
  EXPECT_FUN(parse_type_expr, arg->type);
  if (MATCH_TOK(SEMICOLON)){
    NEXT;
    MATCH_FUN(parse_arg_list, arg->next);
  }
  else
    arg->next = NULL;

  printf("returning from parse_arg_list\n");
  printf("%s\n", BEGET->val);
  PARSE_RETURN(arg);
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


static type_t parse_type(PARSE_PARAMS) {
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
  else if (MATCH_TOK(LIST))
    ty->kind = LIST_TY;
  /*
     else if (MATCH_TOK(MAP))
     ty_>kind = MAP_TY;
     else if (MATCH_TOK(SET))
     ty->kind = SET_TY;
     */  
  else if (MATCH_TOK(IDENTIFIER))
    ty->kind = NAME_TY;

  PARSE_RETURN(ty);

}

static morph_chain_t parse_morph_chain(PARSE_PARAMS) {
  morph_chain_t morph = malloc(sizeof(morph_t));

  if (MATCH_TOK(DOT_DOT_DOT)){
    morph->path = DIRECT_PATH;
    NEXT;
    EXPECT_FUN(parse_type, morph->ty);
  }
  else if (MATCH_TOK(ARROW)){
    morph->path = BEST_PATH;
    NEXT;
    EXPECT_FUN(parse_type, morph->ty);
  }
  else
    EXPECT_TOK(DOT_DOT_DOT);

  MATCH_FUN(parse_morph_chain, morph->next);
  PARSE_RETURN(morph);
}

static type_t parse_type_expr(PARSE_PARAMS) {
  type_t ty = malloc(sizeof(type_t));

  //check for a morph chain
  if (MATCH_FUN(parse_morph_chain, ty->u.morph_ty)){
    ty->kind = MORPH_TY;
  }
  // else parse single type
  else{
    MATCH_FUN(parse_type, ty);
  }
  PARSE_RETURN(ty);
}

static fun_t parse_fun_decl(PARSE_PARAMS) {
  fun_t fun = malloc(sizeof(fun_t));

  fun->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(LPAREN);
  MATCH_FUN(parse_arg_list, fun->args);
  printf("returned from parse_arg_list\n");
  printf("%s\n", BEGET->val);
  EXPECT_TOK(RPAREN);
  printf("parsed RPAREN\n");
  printf("%s\n", BEGET->val);

  if (MATCH_TOK(COLON)){
    NEXT;
    EXPECT_FUN(parse_type_expr, fun->ret_type);
  }else
    fun->ret_type = NULL;

  MATCH_FUN(parse_decl, fun->decl);
  log("return from parse_decl\n");
  log("%s\n", BEGET->val);

  EXPECT_TOK(BEGIN);

  // parse statements
  log("parsed BEGIN\n");
  log("%s\n", BEGET->val);
  EXPECT_TOK(NUF);
  EXPECT_TOK(IDENTIFIER);
  MATCH_FUN(parse_fun_decl, fun->next);
  PARSE_RETURN(fun);
}

// Assignment used in a declaration context
static assign_t parse_static_assign(PARSE_PARAMS) {
  assign_t assign;
  assign = malloc(sizeof(struct assign_st));

  if (MATCH_TOK(EQ)) {
    assign->kind = SIMPLE_AS;
    NEXT;
  }
  else if (MATCH_TOK(COLON_EQ)) {
    assign->kind = DEEP_AS;
    NEXT;
  }
  else {
    PARSE_FAIL("Expected one of '=' or ':=' in static assignment");
  }

  EXPECT_FUN(parse_type_expr, assign->expr);

  PARSE_RETURN(assign);
}


// Assignment used in a statement context
static assign_t parse_assign(PARSE_PARAMS) {
  assign_t assign;
  assign = malloc(sizeof(struct assign_st));

  if (MATCH_TOK(EQ)) {
    assign->kind = SIMPLE_AS;
    NEXT;
  }
  else if (MATCH_TOK(COLON_EQ)) {
    assign->kind = DEEP_AS;
    NEXT;
  }
  else if (MATCH_TOK(PLUS_EQ)) {
    assign->kind = PLUS_AS;
    NEXT;
  }
  else if (MATCH_TOK(MINUS_EQ)) {
    assign->kind = MINUS_AS;
    NEXT;
  }
  else if (MATCH_TOK(MUL_EQ)) {
    assign->kind = MUL_AS;
    NEXT;
  }
  else if (MATCH_TOK(DIV_EQ)) {
    assign->kind = DIV_AS;
    NEXT;
  }
  else if (MATCH_TOK(MOD_EQ)) {
    assign->kind = MOD_AS;
    NEXT;
  }
  else if (MATCH_TOK(OR_EQ)) {
    assign->kind = OR_AS;
    NEXT;
  }
  else if (MATCH_TOK(AND_EQ)) {
    assign->kind = AND_AS;
    NEXT;
  }
  else if (MATCH_TOK(XOR_EQ)) {
    assign->kind = XOR_AS;
    NEXT;
  }
  else {
    PARSE_FAIL("Expected an assignment operator");
  }

  EXPECT_FUN(parse_type_expr, assign->expr);

  PARSE_RETURN(assign);
}


static id_list_t parse_id_list(PARSE_PARAMS) {
  id_list_t id_list;

  id_list = malloc(sizeof(id_list_st));

  id_list->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  if (MATCH_TOK(COMMA)) {
    NEXT;
    EXPECT_FUN(parse_id_list, id_list->next);
  }
  else {
    id_list->next = NULL;
  }

  return id_list;
}


static var_decl_t parse_vars_decl(PARSE_PARAMS) {
  var_decl_t var;
  var = malloc(sizeof(struct var_decl_st));

  var->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  if (MATCH_TOK(COMMA)) {
    NEXT;
    MATCH_FUN(parse_vars_decl, var->next);
  }

  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type, var->type);

  MATCH_FUN(parse_static_assign, var->assign);

  EXPECT_TOK(SEMICOLON);

  MATCH_FUN(parse_vars_decl, var->next);

  PARSE_RETURN(var);
}

// parse constant declarations
static const_decl_t parse_const_decl(PARSE_PARAMS) {
  const_decl_t con;
  con = malloc(sizeof(struct const_decl_st));

  con->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type, con->ty);

  EXPECT_FUN(parse_static_assign, con->assign);

  EXPECT_TOK(SEMICOLON);

  MATCH_FUN(parse_const_decl, con->next);

  PARSE_RETURN(con);
}

// parse type declarations
static type_decl_t parse_type_decl(PARSE_PARAMS) {
  type_decl_t ty = malloc(sizeof(struct type_decl_st));

  ty->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  EXPECT_TOK(EQ);
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

// parse declarations block
static decl_t parse_decl(PARSE_PARAMS) {
  decl_t decl = malloc(sizeof(struct decl_st));

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
    MATCH_FUN(parse_fun_decl, decl->funs);

    log("returned from parse_fun_decl\n");
    log("%s\n", BEGET->val);
  }

  PARSE_RETURN(decl);
}

// parse module decalarations
static mod_t parse_module_decl(PARSE_PARAMS) {
  mod_t mod;
  mod = malloc(sizeof(struct mod_st));

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
  root = malloc(sizeof(struct root_st));

  EXPECT_FUN(parse_module_decl, root->mods);
  log("Parse ended successfully\n");
  return root;
}

