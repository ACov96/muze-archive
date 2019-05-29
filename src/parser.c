#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "util.h"
#include "limits.h"

#define LL_NAME tokens
#define LL_OUT tok_out
#define LEVEL_SPECIFIER parse_level

#define MATCH_FUN(fn, res) \
  (res = fn(LL_NAME, &LL_NAME, LEVEL_SPECIFIER + 1))

#define MATCH_TOK(t) \
  ((BEGET->tok == t) && (NEXT))

#define EXPECT_FUN(fn, res) \
  if (!MATCH_FUN(fn, res)) { \
    return NULL; \
  }

#define EXPECT_TOK(t) \
  do { \
    int _line = BEGET->line_no; \
    int _col = BEGET->col_no; \
    if (!MATCH_TOK(t)) { \
      PARSE_FAIL_AT(_line, _col, \
                    "Expected %s, got %s: '%s'", token_names[t].pretty, \
                    token_names[BEGET->tok].pretty, BEGET->val); \
    } \
  } while(0)

#define PARSE_RETURN(ret) \
  fail_info.level = -1; \
  *LL_OUT = LL_NAME; \
  return ret

#define PARSE_FAIL_AT(_line, _col, ...) \
  if (LEVEL_SPECIFIER > fail_info.level) { \
    snprintf(fail_info.msg, MAX_ERROR_SIZE, __VA_ARGS__); \
    fail_info.line = (_line); \
    fail_info.column = (_col); \
    fail_info.level = LEVEL_SPECIFIER; \
  } \
  return NULL

#define PARSE_FAIL(...) \
  PARSE_FAIL_AT(BEGET->line_no, BEGET->col_no, __VA_ARGS__)

#define PARSE_ASSERT(pred, title, msg, ...) \
  if (!(pred)) { \
    append_error("/to/do", BEGET->line_no, BEGET->col_no, \
                 (title), (msg), ##__VA_ARGS__); \
  }

// Gets the current token
#define BEGET \
  ((token_t)(LL_NAME->val))

#define NEXT \
  LL_NAME = LL_NAME->next

#define PARSE_PARAMS ll_t LL_NAME, ll_t *LL_OUT, int LEVEL_SPECIFIER

// most recent mismatch
static struct {
  char msg[MAX_ERROR_SIZE];
  int line;
  int column;
  int level;
} fail_info;

static void init_fail() {
  fail_info.msg[0] = '\0';
  fail_info.line = -1;
  fail_info.column = -1;
  fail_info.level = -1;
}

// Prototypes
static root_t parse_root(PARSE_PARAMS);
static decl_t parse_decl(PARSE_PARAMS);
static mod_t parse_module_decl(PARSE_PARAMS);
static type_decl_t parse_type_decl(PARSE_PARAMS);
static const_decl_t parse_const_decl(PARSE_PARAMS);
static fun_decl_t parse_fun_decl(PARSE_PARAMS);
static var_decl_t parse_vars_decl(PARSE_PARAMS);
static expr_t parse_expr(PARSE_PARAMS);
static morph_chain_t parse_morph_chain(PARSE_PARAMS);
static literal_t parse_literal(PARSE_PARAMS);
static type_t parse_type_expr(PARSE_PARAMS);
static arg_t parse_arg_list(PARSE_PARAMS);
static char *parse_right_identifier(PARSE_PARAMS);

static expr_t parse_expr(PARSE_PARAMS) {
  expr_t ex = malloc(sizeof(expr_t));

  if (MATCH_FUN(parse_right_identifier, ex->u.id_ex)) {
    ex->kind = ID_EX;
  }
  else if (MATCH_FUN(parse_literal, ex->u.literal_ex)) {
    ex->kind = LITERAL_EX;
  }
  else {
    PARSE_FAIL("Expression expected");
  }
  // TODO

  PARSE_RETURN(ex);
}

static arg_t parse_arg_list(PARSE_PARAMS) {
  arg_t arg = malloc(sizeof(arg_t));

  arg->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  if (MATCH_TOK(COMMA)) {
    EXPECT_FUN(parse_arg_list, arg->next);
  }
  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type_expr, arg->type);
  if (MATCH_TOK(SEMICOLON)){
    MATCH_FUN(parse_arg_list, arg->next);
  }
  else
    arg->next = NULL;

  PARSE_RETURN(arg);
}

static literal_t parse_literal(PARSE_PARAMS) {
  literal_t lit = malloc(sizeof(literal_t));

  if (MATCH_TOK(STRING_VAL)){
    lit->kind = STRING_LIT;
    lit->u.string_lit = BEGET->val;
  }
  else if (MATCH_TOK(INT_VAL)) {
    lit->kind = INTEGER_LIT;
    lit->u.integer_lit = BEGET->val;
  }
  else if (MATCH_TOK(REAL_VAL)) {
    lit->kind = REAL_LIT;
    lit->u.real_lit = BEGET->val;
  }
  else if (MATCH_TOK(TRUE)){
    lit->kind = BOOLEAN_LIT;
    lit->u.bool_lit = TRUE_BOOL;
  }
  else if (MATCH_TOK(FALSE)){
    lit->kind = BOOLEAN_LIT;
    lit->u.bool_lit = FALSE_BOOL;
  }
  else {
    PARSE_FAIL("Literal expected");
  }

  PARSE_RETURN(lit);

}

// Right identifier, basically anything taht can
static char *parse_right_identifier(PARSE_PARAMS) {
  char *id;

  id = BEGET->val;
  if (!(MATCH_TOK(STRING)
      || MATCH_TOK(INTEGER)
      || MATCH_TOK(REAL)
      || MATCH_TOK(BOOLEAN)
      || MATCH_TOK(ARRAY)
      || MATCH_TOK(LIST)
      || MATCH_TOK(TRUE)
      || MATCH_TOK(FALSE)
      || MATCH_TOK(IDENTIFIER)
      )) {
    PARSE_FAIL("Expected identifier");
  }

  PARSE_RETURN(id);
}


static morph_chain_t parse_morph_chain(PARSE_PARAMS) {
  morph_chain_t morph = malloc(sizeof(struct morph_st));

  if (MATCH_TOK(DOT_DOT_DOT)){
    morph->path = DIRECT_PATH;
    EXPECT_FUN(parse_type_expr, morph->ty);
  }
  else if (MATCH_TOK(ARROW)){
    morph->path = BEST_PATH;
    EXPECT_FUN(parse_type_expr, morph->ty);
  }
  else {
    write_log("got here");
    PARSE_FAIL("Expected to find a morph chain, instead got %s",
               token_names[BEGET->tok].pretty);
  }

  MATCH_FUN(parse_morph_chain, morph->next);
  PARSE_RETURN(morph);
}

static rec_t parse_rec_decl(PARSE_PARAMS) {
  rec_t rec = malloc(sizeof(struct rec_st));

  EXPECT_TOK(REC);
  EXPECT_TOK(CER);

  PARSE_RETURN(rec);
}

static type_t parse_type_expr(PARSE_PARAMS) {
  type_t ty = malloc(sizeof(struct type_st));

  if (MATCH_FUN(parse_right_identifier, ty->u.name_ty)) {
    ty->kind = NAME_TY;
  }
  else if (MATCH_FUN(parse_rec_decl, ty->u.rec_ty)) {
    ty->kind = REC_TY;
  }
  else if (MATCH_FUN(parse_morph_chain, ty->u.morph_ty)) {
    ty->kind = MORPH_TY;
  }
  // TODO add a case for enums
  else {
    write_log("falling through to here, fail level is %d", fail_info.level);
    PARSE_FAIL("Expected to find a type expression, instead got %s",
               token_names[BEGET->tok].pretty);
  }

  PARSE_RETURN(ty);
}

static fun_decl_t parse_fun_decl(PARSE_PARAMS) {
  fun_decl_t fun = malloc(sizeof(struct fun_decl_st));

  fun->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(LPAREN);
  MATCH_FUN(parse_arg_list, fun->args);
  EXPECT_TOK(RPAREN);

  if (MATCH_TOK(COLON)){
    EXPECT_FUN(parse_type_expr, fun->ret_type);
  }else
    fun->ret_type = NULL;

  MATCH_FUN(parse_decl, fun->decl);

  EXPECT_TOK(BEGIN);
  // parse statements
  // MATCH_FUN()
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
  }
  else if (MATCH_TOK(COLON_EQ)) {
    assign->kind = DEEP_AS;
  }
  else {
    PARSE_FAIL("Expected one of '=' or ':=' in static assignment");
  }

  EXPECT_FUN(parse_expr, assign->expr);

  PARSE_RETURN(assign);
}


// Assignment used in a statement context
static assign_t parse_assign(PARSE_PARAMS) {
  assign_t assign;
  assign = malloc(sizeof(struct assign_st));

  if (MATCH_TOK(EQ)) {
    assign->kind = SIMPLE_AS;
  }
  else if (MATCH_TOK(COLON_EQ)) {
    assign->kind = DEEP_AS;
  }
  else if (MATCH_TOK(PLUS_EQ)) {
    assign->kind = PLUS_AS;
  }
  else if (MATCH_TOK(MINUS_EQ)) {
    assign->kind = MINUS_AS;
  }
  else if (MATCH_TOK(MULT_EQ)) {
    assign->kind = MULT_AS;
  }
  else if (MATCH_TOK(DIV_EQ)) {
    assign->kind = DIV_AS;
  }
  else if (MATCH_TOK(MOD_EQ)) {
    assign->kind = MOD_AS;
  }
  else if (MATCH_TOK(OR_EQ)) {
    assign->kind = OR_AS;
  }
  else if (MATCH_TOK(AND_EQ)) {
    assign->kind = AND_AS;
  }
  else if (MATCH_TOK(XOR_EQ)) {
    assign->kind = XOR_AS;
  }
  else {
    PARSE_FAIL("Expected an assignment operator");
  }

  EXPECT_FUN(parse_expr, assign->expr);

  PARSE_RETURN(assign);
}


static id_list_t parse_id_list(PARSE_PARAMS) {
  id_list_t id_list;
  id_list = malloc(sizeof(struct id_list_st));

  id_list->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  if (MATCH_TOK(COMMA)) {
    EXPECT_FUN(parse_id_list, id_list->next);
  }
  else {
    id_list->next = NULL;
  }

  PARSE_RETURN(id_list);
}

// parse type declarations
static type_decl_t parse_type_decl(PARSE_PARAMS) {
  type_decl_t ty = malloc(sizeof(struct type_decl_st));

  ty->name = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  EXPECT_TOK(EQ);
  EXPECT_FUN(parse_type_expr, ty->type);
  EXPECT_TOK(SEMICOLON);

  // MATCH_FUN(parse_morph_decl, ty->morphs);

  if (MATCH_TOK(MU)){
    //some morph stuff
    EXPECT_TOK(UM);
  } 

  MATCH_FUN(parse_type_decl,ty->next);

  PARSE_RETURN(ty);
}

static var_decl_t parse_vars_decl(PARSE_PARAMS) {
  var_decl_t var;
  var = malloc(sizeof(struct var_decl_st));

  EXPECT_FUN(parse_id_list, var->names);
  EXPECT_TOK(COLON);
  EXPECT_FUN(parse_type_expr, var->type);

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
  EXPECT_FUN(parse_type_expr, con->ty);

  EXPECT_FUN(parse_static_assign, con->assign);

  EXPECT_TOK(SEMICOLON);

  MATCH_FUN(parse_const_decl, con->next);

  PARSE_RETURN(con);
}

// parse declarations block
static decl_t parse_decl(PARSE_PARAMS) {
  decl_t decl = malloc(sizeof(struct decl_st));

  if (MATCH_TOK(CONST)){
    EXPECT_FUN(parse_const_decl, decl->constants);
  }

  if (MATCH_TOK(TYPE)){
    EXPECT_FUN(parse_type_decl, decl->types);
  }

  if (MATCH_TOK(VAR)){
    EXPECT_FUN(parse_vars_decl, decl->vars);  
  }

  if (MATCH_TOK(FUN)){
    EXPECT_FUN(parse_fun_decl, decl->funs);
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

  PARSE_ASSERT(!strcmp(BEGET->val, mod->name),
      "Module block terminated by the wrong identifier.",
      "Expected '%s' after 'dom', got '%s'.",
      mod->name, BEGET->val);
  EXPECT_TOK(IDENTIFIER);

  MATCH_FUN(parse_module_decl, mod->next);

  PARSE_RETURN(mod);
}

static root_t parse_root(PARSE_PARAMS) {
  root_t root;
  root = malloc(sizeof(struct root_st));

  EXPECT_FUN(parse_module_decl, root->mods);

  PARSE_RETURN(root);
}

// start parse
root_t parse(ll_t LL_NAME) {
  root_t root;
  int LEVEL_SPECIFIER;

  init_fail();

  LEVEL_SPECIFIER = 0;

  if (MATCH_FUN(parse_root, root)) {
    write_log("Parse ended successfully\n");
  }
  else {
    append_error("/to/do", fail_info.line, fail_info.column,
                 "Syntax error", fail_info.msg);
  }

  return root;
}

