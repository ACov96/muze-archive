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
  parse_log("Attempted parse failed: " __VA_ARGS__); \
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
    parse_log("Assersion failed: " msg, ##__VA_ARGS__); \
    append_error("/to/do", BEGET->line_no, BEGET->col_no, \
                 (title), (msg), ##__VA_ARGS__); \
  }

// Gets the current token
#define BEGET \
  ((token_t)(LL_NAME->val))

#define NEXT \
  LL_NAME = LL_NAME->next

#define PARSE_PARAMS ll_t LL_NAME, ll_t *LL_OUT, int LEVEL_SPECIFIER

#define parse_log(str, ...) \
  write_log("[%d] "str, LEVEL_SPECIFIER, ##__VA_ARGS__)

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

static assign_t parse_assign(PARSE_PARAMS);

static expr_t parse_expr(PARSE_PARAMS);
static expr_t parse_lval(PARSE_PARAMS);


// In order of precedence
static expr_t parse_literal_expr(PARSE_PARAMS);

static expr_t parse_postfix_expr(PARSE_PARAMS);
static expr_t parse_prefix_expr(PARSE_PARAMS);

static expr_t parse_unit_expr(PARSE_PARAMS);
static expr_t parse_unary_expr(PARSE_PARAMS);
static expr_t parse_multiplicative_expr(PARSE_PARAMS);
static expr_t parse_additive_expr(PARSE_PARAMS);
static expr_t parse_bitshift_expr(PARSE_PARAMS);
static expr_t parse_bit_and_expr(PARSE_PARAMS);
static expr_t parse_bit_or_expr(PARSE_PARAMS);
static expr_t parse_bit_xor_expr(PARSE_PARAMS);
static expr_t parse_compare_expr(PARSE_PARAMS);
static expr_t parse_and_expr(PARSE_PARAMS);
static expr_t parse_or_expr(PARSE_PARAMS);
static expr_t parse_xor_expr(PARSE_PARAMS);
static expr_t parse_ternary_expr(PARSE_PARAMS);

static morph_chain_t parse_morph_chain(PARSE_PARAMS);
static literal_t parse_literal(PARSE_PARAMS);
static type_t parse_type_expr(PARSE_PARAMS);
static arg_t parse_arg_list(PARSE_PARAMS);
static char *parse_right_identifier(PARSE_PARAMS);

static stmt_t parse_stmt(PARSE_PARAMS);
static cond_stmt_t parse_cond_stmt(PARSE_PARAMS);
static case_stmt_t parse_case_stmt(PARSE_PARAMS);
static for_stmt_t parse_for_stmt(PARSE_PARAMS);
static loop_stmt_t parse_loop_stmt(PARSE_PARAMS);
static assign_stmt_t parse_assign_stmt(PARSE_PARAMS);
static expr_stmt_t parse_expr_stmt(PARSE_PARAMS);


static stmt_t parse_stmt(PARSE_PARAMS) {
  stmt_t stmt = malloc(sizeof(struct stmt_st));

  if (MATCH_FUN(parse_cond_stmt, stmt->u.cond_stmt)) {
    stmt->kind = COND_STMT;
  }
  else if (MATCH_FUN(parse_case_stmt, stmt->u.case_stmt)) {
    stmt->kind = CASE_STMT;
  }
  else if (MATCH_FUN(parse_for_stmt, stmt->u.for_stmt)) {
    stmt->kind = FOR_STMT;
  }
  else if (MATCH_FUN(parse_loop_stmt, stmt->u.loop_stmt)) {
    stmt->kind = LOOP_STMT;
  }
  else if (MATCH_FUN(parse_assign_stmt, stmt->u.assign_stmt)) {
    stmt->kind = ASSIGN_STMT;
  }
  else if (MATCH_FUN(parse_expr_stmt, stmt->u.expr_stmt)) {
    stmt->kind = EXPR_STMT;
  }
  else {
    PARSE_FAIL("Expected to find a statement, instead found %s",
               token_names[BEGET->tok].pretty);
  }

  if (!MATCH_FUN(parse_stmt, stmt->next)) {
    stmt->next = NULL;
  }

  PARSE_RETURN(stmt);
}

static cond_stmt_t parse_cond_stmt(PARSE_PARAMS) {
  cond_stmt_t cond_stmt = malloc(sizeof(struct cond_stmt_st));

  EXPECT_TOK(IF);
  EXPECT_FUN(parse_expr, cond_stmt->test);
  EXPECT_TOK(THEN);
  MATCH_FUN(parse_stmt, cond_stmt->body);

  cond_stmt_t curr = cond_stmt;

  while (MATCH_TOK(ELIF)) {
    curr->else_stmt = malloc(sizeof(struct stmt_st));

    curr->else_stmt->kind = COND_STMT;
    curr->else_stmt->u.cond_stmt = malloc(sizeof(struct cond_stmt_st));

    curr = curr->else_stmt->u.cond_stmt;

    EXPECT_FUN(parse_expr, curr->test);
    EXPECT_TOK(THEN);
    MATCH_FUN(parse_stmt, curr->body);
  }

  if (MATCH_TOK(ELSE)) {
    MATCH_FUN(parse_stmt, curr->else_stmt);
  }

  EXPECT_TOK(FI);

  PARSE_RETURN(cond_stmt);
}

static case_stmt_t parse_case_stmt(PARSE_PARAMS) {
  case_stmt_t case_stmt = malloc(sizeof(struct case_stmt_st));

  EXPECT_TOK(CASE);
  EXPECT_FUN(parse_expr, case_stmt->test);
  EXPECT_TOK(OF);
  // TODO
  EXPECT_TOK(ESAC);

  PARSE_RETURN(case_stmt);
}

static for_stmt_t parse_for_stmt(PARSE_PARAMS) {
  for_stmt_t for_stmt = malloc(sizeof(struct for_stmt_st));

  EXPECT_TOK(FOR);

  for_stmt->iter = BEGET->val;
  EXPECT_TOK(IDENTIFIER);
  EXPECT_TOK(IN);
  EXPECT_FUN(parse_expr, for_stmt->list);

  EXPECT_TOK(DO);
  MATCH_FUN(parse_stmt, for_stmt->body);
  EXPECT_TOK(ROF);

  PARSE_RETURN(for_stmt);
}

static loop_stmt_t parse_loop_stmt(PARSE_PARAMS) {
  loop_stmt_t loop_stmt = malloc(sizeof(struct loop_stmt_st));

  EXPECT_TOK(LOOP);
  MATCH_FUN(parse_stmt, loop_stmt->body);
  EXPECT_TOK(POOL);

  PARSE_RETURN(loop_stmt);
}

static assign_stmt_t parse_assign_stmt(PARSE_PARAMS) {
  assign_stmt_t assign_stmt = malloc(sizeof(struct assign_stmt_st));

  EXPECT_FUN(parse_lval, assign_stmt->lval);

  EXPECT_FUN(parse_assign, assign_stmt->assign);
  EXPECT_TOK(SEMICOLON);

  PARSE_RETURN(assign_stmt);
}

static expr_stmt_t parse_expr_stmt(PARSE_PARAMS) {
  expr_stmt_t expr_stmt = malloc(sizeof(struct expr_stmt_st));

  EXPECT_FUN(parse_expr, expr_stmt->expr);
  EXPECT_TOK(SEMICOLON);

  PARSE_RETURN(expr_stmt);
}

static expr_t parse_expr(PARSE_PARAMS) {
  expr_t ex;

  EXPECT_FUN(parse_ternary_expr, ex);

  PARSE_RETURN(ex);
}

static expr_t parse_id_expr(PARSE_PARAMS) {
  expr_t id = malloc(sizeof(struct expr_st));
  id->kind = ID_EX;
  id->u.id_ex = BEGET->val;
  EXPECT_TOK(IDENTIFIER);

  PARSE_RETURN(id);
}

static expr_t parse_lval(PARSE_PARAMS) {
  expr_t lval = malloc(sizeof(struct expr_st));

  // TODO: delete this line and finish this function, maybe treat subscripting
  // and property access as "operators"
  EXPECT_FUN(parse_id_expr, lval);
//  if (MATCH_FUN(parse_id_expr, lval)) {
//  }
//  else if (MATCH_FUN(

  PARSE_RETURN(lval);
}


static expr_t parse_unit_expr(PARSE_PARAMS) {
  expr_t unit;
  char *id;
  literal_t literal;

  parse_log("Attempting to parse unit expression");

  if (MATCH_FUN(parse_lval, unit)) {
    parse_log("Unit expression is lvalue");
  }
  else if (MATCH_FUN(parse_literal, literal)) {
    parse_log("Unit expression is literal");
    unit = malloc(sizeof(struct expr_st));
    unit->kind = LITERAL_EX;
    unit->u.literal_ex = literal;
  }
  else if (MATCH_FUN(parse_right_identifier, id)) {
    unit = malloc(sizeof(struct expr_st));
    unit->u.id_ex = id;
    parse_log("Unit expression is right identifier");
    unit->kind = ID_EX;
  }
  else if (MATCH_TOK(LPAREN)) {
    parse_log("Unit expression is nested expression");
    EXPECT_FUN(parse_expr, unit);
    EXPECT_TOK(RPAREN);
  }

  parse_log("Successfully parsed unit expression");

  PARSE_RETURN(unit);
}

static expr_t parse_postfix_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  unary_t unary = malloc(sizeof(struct unary_st));
  expr->kind = UNARY_EX;
  expr->u.unary_ex = unary;

  EXPECT_FUN(parse_lval, unary->expr);

  if (MATCH_TOK(INC)) {
    unary->op = POST_INC_OP;
  }
  else if (MATCH_TOK(DEC)) {
    unary->op = POST_DEC_OP;
  }
  else {
    PARSE_FAIL("Expected postfix operator (++ or --), got %s",
               token_names[BEGET->tok].pretty);
  }

  PARSE_RETURN(expr);
}

static expr_t parse_prefix_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  unary_t unary = malloc(sizeof(struct unary_st));
  expr->kind = UNARY_EX;
  expr->u.unary_ex = unary;

  if (MATCH_TOK(INC)) {
    unary->op = PRE_INC_OP;
  }
  else if (MATCH_TOK(DEC)) {
    unary->op = PRE_DEC_OP;
  }
  else {
    PARSE_FAIL("Expected prefix operator, got %s",
               token_names[BEGET->tok].pretty);
  }

  EXPECT_FUN(parse_lval, unary->expr);

  PARSE_RETURN(expr);
}

static expr_t parse_unary_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  unary_t unary = malloc(sizeof(struct unary_st));
  expr->kind = UNARY_EX;
  expr->u.unary_ex = unary;

  if (MATCH_TOK(NOT)) {
    unary->op = NOT_OP;
    if (!MATCH_FUN(parse_unit_expr, unary->expr)) {
      EXPECT_FUN(parse_unary_expr, unary->expr);
    }
  }
  else if (MATCH_TOK(BIT_NOT)) {
    unary->op = BIT_NOT_OP;
    if (!MATCH_FUN(parse_unit_expr, unary->expr)) {
      EXPECT_FUN(parse_unary_expr, unary->expr);
    }
  }
  else {
    EXPECT_FUN(parse_unit_expr, expr);
  }

  PARSE_RETURN(expr);
}

static expr_t parse_multiplicative_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_unary_expr, binary->left);

  if (MATCH_TOK(MULT)) {
    binary->op = MUL_OP;
    EXPECT_FUN(parse_multiplicative_expr, binary->right);
  }
  else if (MATCH_TOK(DIV)) {
    binary->op = DIV_OP;
    EXPECT_FUN(parse_multiplicative_expr, binary->right);
  }
  else if (MATCH_TOK(MODULO)) {
    binary->op = MOD_OP;
    EXPECT_FUN(parse_multiplicative_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_additive_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_multiplicative_expr, binary->left);

  if (MATCH_TOK(PLUS)) {
    binary->op = PLUS_OP;
    EXPECT_FUN(parse_additive_expr, binary->right);
  }
  else if (MATCH_TOK(MINUS)) {
    binary->op = MINUS_OP;
    EXPECT_FUN(parse_additive_expr, binary->right);
  }
  else {
    expr = binary->left;
  }


  PARSE_RETURN(expr);
}

static expr_t parse_bitshiftl_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_additive_expr, binary->left);

  if (MATCH_TOK(SHIFT_LEFT)) {
    binary->op = SHIFT_LEFT_OP;
    EXPECT_FUN(parse_bitshiftl_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_bitshiftr_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_additive_expr, binary->left);

  if (MATCH_TOK(SHIFT_LEFT)) {
    EXPECT_FUN(parse_bitshiftr_expr, binary->right);
    binary->op = SHIFT_RIGHT_OP;
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_bitshift_expr(PARSE_PARAMS) {
  expr_t expr;

  if (!MATCH_FUN(parse_bitshiftl_expr, expr)
      && !MATCH_FUN(parse_bitshiftr_expr, expr)) {
    PARSE_FAIL("Expected bitshift, got %s", token_names[BEGET->tok].pretty);
  }

  PARSE_RETURN(expr);
}

static expr_t parse_bit_and_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_bitshift_expr, binary->left);

  if (MATCH_TOK(BIT_AND)) {
    binary->op = BIT_AND_OP;
    EXPECT_FUN(parse_bit_and_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_bit_or_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_bit_and_expr, binary->left);

  if (MATCH_TOK(BIT_OR)) {
    binary->op = BIT_OR_OP;
    EXPECT_FUN(parse_bit_or_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_bit_xor_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_bit_or_expr, binary->left);

  if (MATCH_TOK(BIT_XOR)) {
    binary->op = BIT_XOR_OP;
    EXPECT_FUN(parse_bit_xor_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_compare_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_bit_xor_expr, binary->left);

  if (MATCH_TOK(EQ_EQ)) {
    binary->op = EQ_EQ_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else if (MATCH_TOK(LT)) {
    binary->op = LT_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else if (MATCH_TOK(GT)) {
    binary->op = GT_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else if (MATCH_TOK(NOT_EQ)) {
    binary->op = NOT_EQ_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else if (MATCH_TOK(LT_EQ)) {
    binary->op = LT_EQ_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else if (MATCH_TOK(GT_EQ)) {
    binary->op = GT_EQ_OP;
    EXPECT_FUN(parse_compare_expr, binary->right);
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_and_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_compare_expr, binary->left);

  if (MATCH_TOK(AND)) {
    EXPECT_FUN(parse_and_expr, binary->right);
    binary->op = AND_OP;
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_or_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_and_expr, binary->left);

  if (MATCH_TOK(AND)) {
    EXPECT_FUN(parse_compare_expr, binary->right);
    binary->op = OR_OP;
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_xor_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  binary_t binary = malloc(sizeof(struct binary_st));
  expr->kind = BINARY_EX;
  expr->u.binary_ex = binary;

  EXPECT_FUN(parse_or_expr, binary->left);

  if (MATCH_TOK(AND)) {
    EXPECT_FUN(parse_compare_expr, binary->right);
    binary->op = XOR_OP;
  }
  else {
    expr = binary->left;
  }

  PARSE_RETURN(expr);
}

static expr_t parse_ternary_expr(PARSE_PARAMS) {
  expr_t expr = malloc(sizeof(struct expr_st));
  ternary_t ternary = malloc(sizeof(struct ternary_st));
  expr->kind = TERNARY_EX;
  expr->u.ternary_ex = ternary;

  EXPECT_FUN(parse_xor_expr, ternary->left);

  if (MATCH_TOK(QUESTION)) {
    EXPECT_FUN(parse_xor_expr, ternary->middle);
    EXPECT_TOK(COLON);
    EXPECT_FUN(parse_ternary_expr, ternary->right);
  }
  else {
    expr = ternary->left;
  }

  PARSE_RETURN(expr);
}

static arg_t parse_arg_list(PARSE_PARAMS) {
  arg_t arg = malloc(sizeof(struct arg_st));

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
  literal_t lit = malloc(sizeof(struct literal_st));

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
  fun->stmts = NULL;
  MATCH_FUN(parse_stmt, fun->stmts);
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

