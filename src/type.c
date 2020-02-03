#include <stdlib.h>
#include "type.h"
#include "ast.h"
#include "symbol.h"

const type_id STRING_TYPE = 1;
const type_id BOOLEAN_TYPE = 2;
const type_id INTEGER_TYPE = 3;
const type_id REAL_TYPE = 4;
const type_id UNIT_TYPE = 5;
const type_id LIST_TYPE = 6;
const type_id HASHMAP_TYPE = 7;

// If Alex's concat can stay, this can stay
type_id new_type_id() {
  return (type_id) malloc(1);
}

static type_id evaluate_id_type(char *id, scope_t scope) {
  symbol_t symb = find_symbol(scope, id);

  if (symb) {
    if (symb->kind == VAR_SYMB) {
      return symb->u.var.type;
    }
    else {
      // Wrong kind of symbol
      // TODO error message
    }
  }
  else {
    // Symbol not found
    // TODO error message
  }

  return 0;
}

static type_id evaluate_literal_type(literal_t literal) {
  switch (literal->kind) {
    case STRING_LIT:
      return STRING_TYPE;
    case BOOLEAN_LIT:
      return BOOLEAN_TYPE;
    case INTEGER_LIT:
      return INTEGER_TYPE;
    case REAL_LIT:
      return REAL_TYPE;
    case NULL_LIT:
      return UNIT_TYPE;
  }
}

static type_id evaluate_unary_type(unary_t unary) {
  //switch (unary->kind) {
  //}
  return 0;
}

static type_id evaluate_binary_type(binary_t binary) {
  return 0;
}

static type_id evaluate_call_type(call_t call) {
  return 0;
}

type_id evaluate_expr_type(expr_t expr) {
  switch (expr->kind) {
    case ID_EX:
      evaluate_id_type(expr->u.id_ex);
    case LITERAL_EX:
      evaluate_literal_type(expr->u.literal_ex);
    case UNARY_EX:
      evaluate_unary_type(expr->u.unary_ex);
    case BINARY_EX:
      evaluate_binary_type(expr->u.binary_ex);
    case TERNARY_EX:
      evaluate_ternary_type(expr->u.ternary_ex);
    case CALL_EX:
      evaluate_call_type(expr->u.call_ex, expr->scope);
    case RANGE_EX:
    case MORPH_EX:
    default:
      // TODO fail with 'unsupported'
      return NULL;
  }
}

