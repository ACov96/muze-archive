#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "symbol.h"
#include "lexer.h"
#include "util.h"
#include "limits.h"

#define LL_NAME _tokens
#define LL_OUT _tok_out
#define LEVEL_SPECIFIER _parse_level
#define SCOPE _scope

#define ADD_SYMBOL(symbol) \
	add_symbol(SCOPE, (symbol))

#define ADD_SCOPE(stmts) \
	do { \
		scope_t _old_scope = SCOPE; \
		SCOPE = scope_new(_old_scope); \
		stmts; \
		SCOPE = _old_scope; \
	} while (0)

#define MATCH_FUN(fn, res) \
	(res = fn(LL_NAME, &LL_NAME, SCOPE, LEVEL_SPECIFIER + 1))

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
		append_error("__TODO__", BEGET->line_no, BEGET->col_no, \
								 (title), (msg), ##__VA_ARGS__); \
	}

// Gets the current token
#define BEGET \
	((token_t)(LL_NAME->val))

#define NEXT \
	LL_NAME = LL_NAME->next

#define PARSE_PARAMS ll_t LL_NAME, ll_t *LL_OUT, scope_t SCOPE, \
										 int LEVEL_SPECIFIER

#define parse_log(str, ...) \
	if (log_enable) write_log("[%6d] "str, LEVEL_SPECIFIER, ##__VA_ARGS__)

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

// Globals
extern int log_enable;

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
static lval_t parse_lval(PARSE_PARAMS);
static accessor_list_t parse_accessor_list(PARSE_PARAMS);
static array_type_t parse_array_decl(PARSE_PARAMS);
static rec_t parse_rec_decl(PARSE_PARAMS);
static rec_field_t parse_rec_fields(PARSE_PARAMS);
static id_list_t parse_id_list(PARSE_PARAMS);


// In order of precedence
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
static call_t parse_fun_call(PARSE_PARAMS);
static expr_list_t parse_expr_list(PARSE_PARAMS);


static stmt_t parse_stmt(PARSE_PARAMS);
static cond_stmt_t parse_cond_stmt(PARSE_PARAMS);
static case_stmt_t parse_case_stmt(PARSE_PARAMS);
static for_stmt_t parse_for_stmt(PARSE_PARAMS);
static loop_stmt_t parse_loop_stmt(PARSE_PARAMS);
static assign_stmt_t parse_assign_stmt(PARSE_PARAMS);
static expr_stmt_t parse_expr_stmt(PARSE_PARAMS);
static break_stmt_t parse_break_stmt(PARSE_PARAMS);


static stmt_t parse_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse statement");

	stmt_t stmt = malloc(sizeof(struct stmt_st));

	if (MATCH_FUN(parse_cond_stmt, stmt->u.cond_stmt)) {
		parse_log("Statement is 'if'");
		stmt->kind = COND_STMT;
	}
	else if (MATCH_FUN(parse_case_stmt, stmt->u.case_stmt)) {
		parse_log("Statement is 'case'");
		stmt->kind = CASE_STMT;
	}
	else if (MATCH_FUN(parse_for_stmt, stmt->u.for_stmt)) {
		parse_log("Statement is 'for'");
		stmt->kind = FOR_STMT;
	}
	else if (MATCH_FUN(parse_loop_stmt, stmt->u.loop_stmt)) {
		parse_log("Statement is 'loop'");
		stmt->kind = LOOP_STMT;
	}
	else if (MATCH_FUN(parse_assign_stmt, stmt->u.assign_stmt)) {
		parse_log("Statement is assignment");
		stmt->kind = ASSIGN_STMT;
	}
	else if (MATCH_FUN(parse_expr_stmt, stmt->u.expr_stmt)) {
		parse_log("Statement is expression");
		stmt->kind = EXPR_STMT;
	}
	else if (MATCH_FUN(parse_break_stmt, stmt->u.break_stmt)) {
		parse_log("Statement is break");
		stmt->kind = BREAK_STMT;
	}
	else {
		PARSE_FAIL("Expected to find a statement, instead found %s",
							 token_names[BEGET->tok].pretty);
	}

	if (!MATCH_FUN(parse_stmt, stmt->next)) {
		stmt->next = NULL;
	}

	parse_log("Successfully parsed statement");

	PARSE_RETURN(stmt);
}

static cond_stmt_t parse_cond_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse 'if' statement");

	cond_stmt_t cond_stmt = malloc(sizeof(struct cond_stmt_st));

	EXPECT_TOK(IF);
	EXPECT_FUN(parse_expr, cond_stmt->test);
	EXPECT_TOK(THEN);
	MATCH_FUN(parse_stmt, cond_stmt->body);

	cond_stmt_t curr = cond_stmt;

	while (MATCH_TOK(ELIF)) {
		parse_log("Found 'elif' block");

		curr->else_stmt = malloc(sizeof(struct stmt_st));

		curr->else_stmt->kind = COND_STMT;
		curr->else_stmt->u.cond_stmt = malloc(sizeof(struct cond_stmt_st));

		curr = curr->else_stmt->u.cond_stmt;

		EXPECT_FUN(parse_expr, curr->test);
		EXPECT_TOK(THEN);
		MATCH_FUN(parse_stmt, curr->body);
	}

	if (MATCH_TOK(ELSE)) {
		parse_log("Found 'else' block");
		curr->else_stmt = malloc(sizeof(struct stmt_st));
		curr->else_stmt->kind = COND_STMT;
		curr->else_stmt->u.cond_stmt = malloc(sizeof(struct cond_stmt_st));
		curr = curr->else_stmt->u.cond_stmt;
		curr->test = NULL;
		MATCH_FUN(parse_stmt, curr->body);
	}

	EXPECT_TOK(FI);

	parse_log("Successfully parsed 'if' statement");

	PARSE_RETURN(cond_stmt);
}

static case_stmt_t parse_case_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse 'case' statement");

	case_stmt_t case_stmt = malloc(sizeof(struct case_stmt_st));

	EXPECT_TOK(CASE);
	EXPECT_FUN(parse_expr, case_stmt->test);
	EXPECT_TOK(OF);
	// TODO
	EXPECT_TOK(ESAC);

	parse_log("Successfully parsed 'case' statement");

	PARSE_RETURN(case_stmt);
}

static for_stmt_t parse_for_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse 'for' statement");

	for_stmt_t for_stmt = malloc(sizeof(struct for_stmt_st));

	EXPECT_TOK(FOR);

	for_stmt->iter = BEGET->val;
	EXPECT_TOK(IDENTIFIER);
	EXPECT_TOK(IN);
	EXPECT_FUN(parse_expr, for_stmt->list);

	EXPECT_TOK(DO);
	MATCH_FUN(parse_stmt, for_stmt->body);
	EXPECT_TOK(ROF);

	parse_log("Successfully parsed 'for' statement");

	PARSE_RETURN(for_stmt);
}

static loop_stmt_t parse_loop_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse 'loop' statement");

	loop_stmt_t loop_stmt = malloc(sizeof(struct loop_stmt_st));

	EXPECT_TOK(LOOP);
	MATCH_FUN(parse_stmt, loop_stmt->body);
	EXPECT_TOK(POOL);

	parse_log("Successfully parsed 'loop' statement");

	PARSE_RETURN(loop_stmt);
}

static assign_stmt_t parse_assign_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse assignment statement");

	assign_stmt_t assign_stmt = malloc(sizeof(struct assign_stmt_st));

	EXPECT_FUN(parse_lval, assign_stmt->lval);

	EXPECT_FUN(parse_assign, assign_stmt->assign);
	EXPECT_TOK(SEMICOLON);

	parse_log("Successfully parsed assignment statement");

	PARSE_RETURN(assign_stmt);
}

static expr_stmt_t parse_expr_stmt(PARSE_PARAMS) {
	parse_log("Attempting to parse expression statement");

	expr_stmt_t expr_stmt = malloc(sizeof(struct expr_stmt_st));

	EXPECT_FUN(parse_expr, expr_stmt->expr);
	EXPECT_TOK(SEMICOLON);

	parse_log("Successfully parsed expression statement");

	PARSE_RETURN(expr_stmt);
}

static expr_t parse_expr(PARSE_PARAMS) {
	parse_log("Attempting to parse expression");

	expr_t ex = malloc(sizeof(struct expr_st));

	EXPECT_FUN(parse_ternary_expr, ex);

	parse_log("Successfully parsed expression");

	PARSE_RETURN(ex);
}

static expr_t parse_id_expr(PARSE_PARAMS) {
	parse_log("Attempting to parse identifier expression");

	expr_t id = malloc(sizeof(struct expr_st));

	id->kind = ID_EX;
	id->u.id_ex = BEGET->val;
	EXPECT_TOK(IDENTIFIER);

	parse_log("Successfully parsed identifier expression");

	PARSE_RETURN(id);
}

static lval_t parse_lval(PARSE_PARAMS) {
	parse_log("Attempting to parse lvalue");

	lval_t lval = malloc(sizeof(struct expr_st));
	
	// Parse the id expression part of the lval
	EXPECT_FUN(parse_id_expr, lval->expr);

	// check for accessors
	MATCH_FUN(parse_accessor_list, lval->accessors);

	PARSE_RETURN(lval);
}

static accessor_list_t parse_accessor_list(PARSE_PARAMS) {
	parse_log("Attempting to parse accessor list");

	accessor_list_t acc_list = malloc(sizeof(struct accessor_list_st));

	if (MATCH_TOK(LPAREN)) {
		parse_log("access is subscript.");
		acc_list->kind = SUBSCRIPT;
		EXPECT_FUN(parse_expr, acc_list->u.subscript_expr);
		EXPECT_TOK(RPAREN);
	}
	else if (MATCH_TOK(DOT)) {
		parse_log("accessor is field.");
		acc_list->kind = FIELD;
		acc_list->u.field_id = BEGET->val;
	}
	else{
		return NULL;
	}

	MATCH_FUN(parse_accessor_list, acc_list->next);

	parse_log("Successfully parsed accessor list");
	
	PARSE_RETURN(acc_list);
}

static expr_t parse_unit_expr(PARSE_PARAMS) {
	parse_log("Attempting to parse unit expression");

	expr_t    unit    = NULL;
	char      *id     = NULL;
	literal_t literal = NULL;
	call_t    call    = NULL;

	/* if (MATCH_FUN(parse_lval, unit)) { */
	/*   parse_log("Unit expression is lvalue"); */
	/* } */
	/* else  */
	if (MATCH_FUN(parse_literal, literal)) {
		parse_log("Unit expression is literal");
		unit = malloc(sizeof(struct expr_st));
		unit->kind = LITERAL_EX;
		unit->u.literal_ex = literal;
	}
	else if (MATCH_FUN(parse_right_identifier, id)) {
		unit = malloc(sizeof(struct expr_st));

		/* Sorry, Rory, I did a lazy. - Alex
		 *
		 * This function call stuff should probably be in parse_lval, but it was 
		 * easier to do it here for the time being.
		 *
		 * I forgive you -- Rory
		 *
		 * TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
		 */
		if (MATCH_TOK(LPAREN)) {
			// Function call
			MATCH_FUN(parse_fun_call, call);
			call->id = id;
			unit->kind = CALL_EX;
			unit->u.call_ex = call;
			parse_log("Unit expression is function call");
		} else {
			// Just an identifier
			unit->kind = ID_EX;
			unit->u.id_ex = id;
			parse_log("Unit expression is right identifier");
		}
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
	parse_log("Attempting to parse postfix expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	unary_t unary = malloc(sizeof(struct unary_st));
	expr->kind = UNARY_EX;
	expr->u.unary_ex = unary;

	EXPECT_FUN(parse_expr, unary->expr);

	if (MATCH_TOK(INC)) {
		parse_log("Postfix expression is increment");
		unary->op = POST_INC_OP;
	}
	else if (MATCH_TOK(DEC)) {
		parse_log("Postfix expression is decrement");
		unary->op = POST_DEC_OP;
	}
	else {
		PARSE_FAIL("Expected postfix operator (++ or --), got %s",
							 token_names[BEGET->tok].pretty);
	}

	parse_log("Successfully parsed postfix expression");

	PARSE_RETURN(expr);
}

static expr_t parse_prefix_expr(PARSE_PARAMS) {
	parse_log("Attempting to parse prefix expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	unary_t unary = malloc(sizeof(struct unary_st));
	expr->kind = UNARY_EX;
	expr->u.unary_ex = unary;

	if (MATCH_TOK(INC)) {
		parse_log("Prefix expression is increment");
		unary->op = PRE_INC_OP;
	}
	else if (MATCH_TOK(DEC)) {
		parse_log("Prefix expression is decrement");
		unary->op = PRE_DEC_OP;
	}
	else {
		PARSE_FAIL("Expected prefix operator, got %s",
							 token_names[BEGET->tok].pretty);
	}

	EXPECT_FUN(parse_expr, unary->expr);

	parse_log("Successfully parsed prefix expression");

	PARSE_RETURN(expr);
}

static expr_t parse_unary_expr(PARSE_PARAMS) {
	parse_log("Attempting to parse unary expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	unary_t unary = malloc(sizeof(struct unary_st));
	expr->kind = UNARY_EX;
	expr->u.unary_ex = unary;

	if (MATCH_TOK(NOT)) {
		parse_log("Unary expression is 'not'");

		unary->op = NOT_OP;
		if (!MATCH_FUN(parse_unit_expr, unary->expr)) {
			// TODO: Pretty sure this line is incorrect, as I don't understand it at
			//       all. Test unary expressions ASAP
			EXPECT_FUN(parse_unary_expr, unary->expr);
		}
	}
	else if (MATCH_TOK(BIT_NOT)) {
		parse_log("Unary expression is bitwise 'not'");

		unary->op = BIT_NOT_OP;
		if (!MATCH_FUN(parse_unit_expr, unary->expr)) {
			// TODO: Same here
			EXPECT_FUN(parse_unary_expr, unary->expr);
		}
	}
	else {
		parse_log("No unary expression found, continuing expression parse");
		EXPECT_FUN(parse_unit_expr, expr);
	}

	parse_log("Successfully parsed expression");

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

	if (MATCH_TOK(SHIFT_RIGHT)) {
		EXPECT_FUN(parse_bitshiftr_expr, binary->right);
		binary->op = SHIFT_RIGHT_OP;
	}
	else {
		expr = binary->left;
	}

	PARSE_RETURN(expr);
}

static expr_t parse_bitshift_expr(PARSE_PARAMS) {
	expr_t expr = malloc(sizeof(struct expr_st));

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
	parse_log("Parsing 'and' expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	binary_t binary = malloc(sizeof(struct binary_st));
	expr->kind = BINARY_EX;
	expr->u.binary_ex = binary;

	EXPECT_FUN(parse_compare_expr, binary->left);

	if (MATCH_TOK(AND)) {
		parse_log("'and' found");
		EXPECT_FUN(parse_and_expr, binary->right);
		binary->op = AND_OP;
	}
	else {
		parse_log("'and' not found");
		expr = binary->left;
	}

	parse_log("Successfully parsed 'and' expression");

	PARSE_RETURN(expr);
}

static expr_t parse_or_expr(PARSE_PARAMS) {
	parse_log("Parsing 'or' expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	binary_t binary = malloc(sizeof(struct binary_st));
	expr->kind = BINARY_EX;
	expr->u.binary_ex = binary;

	EXPECT_FUN(parse_and_expr, binary->left);

	if (MATCH_TOK(OR)) {
		parse_log("'or' found");
		EXPECT_FUN(parse_compare_expr, binary->right);
		binary->op = OR_OP;
	}
	else {
		parse_log("'or' not found");
		expr = binary->left;
	}

	parse_log("Successfully parsed 'or' expression");

	PARSE_RETURN(expr);
}

static expr_t parse_xor_expr(PARSE_PARAMS) {
	parse_log("Parsing 'xor' expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	binary_t binary = malloc(sizeof(struct binary_st));
	expr->kind = BINARY_EX;
	expr->u.binary_ex = binary;

	EXPECT_FUN(parse_or_expr, binary->left);

	if (MATCH_TOK(XOR)) {
		parse_log("'xor' found");
		EXPECT_FUN(parse_compare_expr, binary->right);
		binary->op = XOR_OP;
	}
	else {
		parse_log("'xor' not found");
		expr = binary->left;
	}

	parse_log("Successfully parsed xor expression");

	PARSE_RETURN(expr);
}

static expr_t parse_ternary_expr(PARSE_PARAMS) {
	parse_log("Parsing ternary expression");

	expr_t expr = malloc(sizeof(struct expr_st));
	ternary_t ternary = malloc(sizeof(struct ternary_st));
	expr->kind = TERNARY_EX;
	expr->u.ternary_ex = ternary;

	EXPECT_FUN(parse_xor_expr, ternary->left);

	if (MATCH_TOK(QUESTION)) {
		parse_log("Ternary found");
		EXPECT_FUN(parse_xor_expr, ternary->middle);
		EXPECT_TOK(COLON);
		EXPECT_FUN(parse_ternary_expr, ternary->right);
	}
	else {
		parse_log("Ternary not found");
		expr = ternary->left;
	}

	parse_log("Successfully parsed ternary expression");

	PARSE_RETURN(expr);
}

static arg_t parse_arg_list(PARSE_PARAMS) {
	arg_t arg = malloc(sizeof(struct arg_st));

	arg->name = BEGET->val;
	EXPECT_TOK(IDENTIFIER);

	// TODO make this (and all symbol additions) have their own kind
	ADD_SYMBOL(symbol_new(arg->name));

	if (MATCH_TOK(COMMA)) {
		EXPECT_FUN(parse_arg_list, arg->next);
	}

	if (MATCH_TOK(COLON)) {
		EXPECT_FUN(parse_type_expr, arg->type);
	}

	if (MATCH_TOK(SEMICOLON)){
		MATCH_FUN(parse_arg_list, arg->next);
	}
	else {
		arg->next = NULL;
	}

	PARSE_RETURN(arg);
}

static literal_t parse_literal(PARSE_PARAMS) {
	literal_t lit = malloc(sizeof(struct literal_st));

	switch(BEGET->tok) {
	case STRING_VAL:
		lit->kind = STRING_LIT;
		lit->u.string_lit = BEGET->val;
		EXPECT_TOK(STRING_VAL);
		break;
	case INT_VAL:
		lit->kind = INTEGER_LIT;
		lit->u.integer_lit = BEGET->val;
		EXPECT_TOK(INT_VAL);
		break;
	case REAL_VAL:
		lit->kind = REAL_LIT;
		lit->u.real_lit = BEGET->val;
		EXPECT_TOK(REAL_VAL);
		break;
	case NULL_VAL:
		lit->kind = NULL_LIT;
		EXPECT_TOK(NULL_VAL);
		break;
	case TRUE:
		lit->kind = BOOLEAN_LIT;
		lit->u.bool_lit = TRUE_BOOL;
		EXPECT_TOK(TRUE);
		break;
	case FALSE:
		lit->kind = BOOLEAN_LIT;
		lit->u.bool_lit = FALSE_BOOL;
		EXPECT_TOK(FALSE);
		break;
	// array literal
	case LBRACKET:
		EXPECT_TOK(LBRACKET);
		EXPECT_FUN(parse_expr_list, lit->u.array_lit);
		lit->kind = ARRAY_LIT;
		EXPECT_TOK(RBRACKET);
		break;
	// record literal
	case LBRACE:
		EXPECT_TOK(LBRACE);
		EXPECT_FUN(parse_expr_list, lit->u.record_lit);
		lit->kind = RECORD_LIT;
		EXPECT_TOK(RBRACE);
		break;
	default:
		PARSE_FAIL("Literal expected");
	}
	PARSE_RETURN(lit);
}

// Right identifier, basically anything taht can
static char *parse_right_identifier(PARSE_PARAMS) {
	char *id = NULL;

	id = BEGET->val;
	if (!(MATCH_TOK(STRING)
			|| MATCH_TOK(INTEGER)
			|| MATCH_TOK(REAL)
			|| MATCH_TOK(BOOLEAN)
			|| MATCH_TOK(LIST)
			|| MATCH_TOK(TRUE)
			|| MATCH_TOK(FALSE)
			|| MATCH_TOK(IDENTIFIER)
			)) {
		PARSE_FAIL("Expected identifier, got %s", id);
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
	parse_log("Attempting to parse record");
	rec_t rec = malloc(sizeof(struct rec_st));

	EXPECT_TOK(REC);
	EXPECT_FUN(parse_rec_fields, rec->fields);
	EXPECT_TOK(CER);

	parse_log("Successfully parsed record");
	PARSE_RETURN(rec);
}

static rec_field_t parse_rec_fields(PARSE_PARAMS) {
	parse_log("Attempting to parse record fields");
	
	rec_field_t rec_field = malloc(sizeof(struct rec_field_st));

	EXPECT_FUN(parse_id_list, rec_field->name);
	EXPECT_TOK(COLON);
	EXPECT_FUN(parse_type_expr, rec_field->type);
	EXPECT_TOK(SEMICOLON);

	MATCH_FUN(parse_rec_fields, rec_field->next);

	parse_log("Successfully parsed record fields");

	PARSE_RETURN(rec_field);
}

static array_type_t parse_array_decl(PARSE_PARAMS){
	array_type_t arr = malloc(sizeof(struct array_type_st));

	EXPECT_TOK(ARRAY);
	if (MATCH_TOK(OF))
		EXPECT_FUN(parse_type_expr, arr->type);

	PARSE_RETURN(arr);
}

static type_t parse_type_expr(PARSE_PARAMS) {
	type_t ty = malloc(sizeof(struct type_st));

	if (MATCH_FUN(parse_right_identifier, ty->u.name_ty)) {
		ty->kind = NAME_TY;
	}
	else if (MATCH_FUN(parse_array_decl, ty->u.array_ty)){
		ty->kind = ARRAY_TY;
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

	fun->symbol = symbol_new(fun->name);

	ADD_SCOPE(
			EXPECT_TOK(LPAREN);
			MATCH_FUN(parse_arg_list, fun->args);
			EXPECT_TOK(RPAREN);

			if (MATCH_TOK(COLON)) {
				EXPECT_FUN(parse_type_expr, fun->ret_type);
			} else {
				fun->ret_type = NULL;
			}

			MATCH_FUN(parse_decl, fun->decl);

			EXPECT_TOK(BEGIN);

			// parse statements
			fun->stmts = NULL;
			MATCH_FUN(parse_stmt, fun->stmts);
			EXPECT_TOK(NUF);

			char *id = BEGET->val;
			EXPECT_TOK(IDENTIFIER);
			PARSE_ASSERT(!strcmp(id, fun->name),
					"Module block terminated by the wrong identifier.",
					"Expected '%s' after 'dom', got '%s'.",
					fun->name, id);
		);

	ADD_SYMBOL(fun->symbol);

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
		PARSE_FAIL("Expected one of '=' or ':=' in static assignment, but got '%s' instead", BEGET->val);
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

	ADD_SYMBOL(symbol_new(id_list->name));
	
	if (MATCH_TOK(COMMA)) {
		EXPECT_FUN(parse_id_list, id_list->next);
	}
	else {
		id_list->next = NULL;
	}

	PARSE_RETURN(id_list);
}

static morph_t parse_morph_decl(PARSE_PARAMS) {
	morph_t morph = malloc(sizeof(struct morph_st));

	EXPECT_TOK(MU);

	morph->target = BEGET->val;
	EXPECT_TOK(IDENTIFIER);

	EXPECT_FUN(parse_stmt, morph->defn);

	EXPECT_TOK(UM);

	char *closing_id = BEGET->val;
	EXPECT_TOK(IDENTIFIER);

	PARSE_ASSERT(!strcmp(closing_id, morph->target),
			"Morph block terminated by the wrong identifier",
			"Expected %s after 'um', got %s", morph->target, closing_id);

	morph->next = NULL;
	MATCH_FUN(parse_morph_decl, morph->next);

	PARSE_RETURN(morph);
}

// parse type declarations
static type_decl_t parse_type_decl(PARSE_PARAMS) {
	type_decl_t ty = malloc(sizeof(struct type_decl_st));

	ty->name = BEGET->val;
	EXPECT_TOK(IDENTIFIER);

	EXPECT_TOK(EQ);
	EXPECT_FUN(parse_type_expr, ty->type);
	EXPECT_TOK(SEMICOLON);

	MATCH_FUN(parse_morph_decl, ty->morphs);

	// NOTE: do we possibly want types to have their own local scope?
	ADD_SYMBOL(symbol_new(ty->name));

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

	mod->symbol = symbol_new(mod->name);

	ADD_SCOPE(
			MATCH_FUN(parse_decl, mod->decl);

			mod->stmts = NULL;

			if (MATCH_TOK(BEGIN)) {
				MATCH_FUN(parse_stmt, mod->stmts);
			}

			EXPECT_TOK(DOM);

			char *id = BEGET->val;
			EXPECT_TOK(IDENTIFIER);

			PARSE_ASSERT(!strcmp(id, mod->name),
					"Module block terminated by the wrong identifier.",
					"Expected '%s' after 'dom', got '%s'.",
					mod->name, id);
		);

	ADD_SYMBOL(mod->symbol);

	// Modules do not inherit their parent scope
	mod->symbol->scope->parent = NULL;

	MATCH_FUN(parse_module_decl, mod->next);

	PARSE_RETURN(mod);
}

static root_t parse_root(PARSE_PARAMS) {
	root_t root;
	root = malloc(sizeof(struct root_st));

	EXPECT_FUN(parse_module_decl, root->mods);

	PARSE_RETURN(root);
}

static call_t parse_fun_call(PARSE_PARAMS) {
	call_t call = malloc(sizeof(struct call_st));

	// The identifier for this call was parsed before, so it'll be populated after this returns
	if (MATCH_TOK(RPAREN))
		call->args = NULL;
	else {
		MATCH_FUN(parse_expr_list, call->args);
		EXPECT_TOK(RPAREN);
	}
	PARSE_RETURN(call);
}

static expr_list_t parse_expr_list(PARSE_PARAMS) {
	expr_list_t curr = NULL;
	if (!MATCH_TOK(RPAREN)) {
		curr = malloc(sizeof(struct expr_list_st));
		MATCH_FUN(parse_expr, curr->expr);
		if (MATCH_TOK(COMMA)) {
			MATCH_FUN(parse_expr_list, curr->next);
		}
	}
	PARSE_RETURN(curr);
}

static break_stmt_t parse_break_stmt(PARSE_PARAMS) {
	EXPECT_TOK(BREAK);

	// If we ever do break to a specific label, this is where we're gonna do it
	
	EXPECT_TOK(SEMICOLON);
	break_stmt_t break_stmt = malloc(sizeof(struct break_stmt_st));
	PARSE_RETURN(break_stmt);
}

// start parse
root_t parse(ll_t LL_NAME) {
	root_t root = malloc(sizeof(struct root_st));
	int LEVEL_SPECIFIER;
	scope_t SCOPE = scope_new(NULL);

	init_fail();

	LEVEL_SPECIFIER = 0;

	if (MATCH_FUN(parse_root, root)) {
		write_log("Parse ended successfully\n");
	}
	else {
		append_error("/to/do", fail_info.line, fail_info.column,
								 "Syntax error", fail_info.msg);
	}

	printf("parse ended successfully\n");
	return root;
}

