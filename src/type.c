#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "morph_graph.h"
#include "symbol.h"


#define TYPE_C
#include "type.h"

static type_id evaluate_expr_type(expr_t expr);

static type_node_t *GRAPH = NULL;

// hack to get the path length for now
static int path_length(char **path) {
  int i;
  for (i = 0; path[i]; i++);
  return i;
}

#define TYPE_ERROR(msg)

type_id morph_or_error(type_id from, type_id to[], int len) {
  int s_dist = -1;
  type_id s_ty = -1;

  if (from == -1) {
    return -1;
  }

  char *froms = get_type_name(GRAPH, from);

  for (int i = 0; i < len; i++) {
    char *tos = get_type_name(GRAPH, to[i]);

    int dist = path_length(shortest_path(GRAPH, froms, tos));

    if (s_dist == -1) {
      s_dist = dist;
      s_ty = to[i];
    }
    else if (dist <= s_dist) {
      s_dist = dist;
      s_ty = to[i];
    }
  }

  // check if we failed to find a morph
  if (s_ty == -1) {
    fprintf(stderr, "Type check from %s failed, could not find morph\n",
            froms);
  }

  return s_ty;
}

#define MORPH_OR_ERROR(from, ...) \
  morph_or_error(from, (type_id[]){__VA_ARGS__}, \
                 sizeof((type_id[]){__VA_ARGS__}))

type_id INTEGER_TYPE = 0;
type_id REAL_TYPE = 0;
type_id BOOLEAN_TYPE = 0;
type_id STRING_TYPE = 0;

static void init_types(type_node_t *graph) {
  INTEGER_TYPE = get_type_index(graph, "integer");
  REAL_TYPE = get_type_index(graph, "real");
  BOOLEAN_TYPE = get_type_index(graph, "boolean");
  STRING_TYPE = get_type_index(graph, "string");

  GRAPH = graph;
}

static type_id evaluate_id_type(char *id, scope_t scope) {
  symbol_t symb = find_symbol(scope, id);

  if (symb) {
    if (symb->kind == VAR_SYMB) {
      return symb->u.var.type;
    }
    else {
      // Wrong kind of symbol
      fprintf(stderr, "Error: symbol %s is not a variable\n", id);
    }
  }
  else {
    // Symbol not found
    fprintf(stderr, "Error: symbol %s not found\n", id);
  }

  return -1;
}

static type_id evaluate_call_type(call_t call, scope_t scope) {
  symbol_t symb = find_symbol(scope, call->id);

  if (symb) {
    if (symb->kind == FUNC_SYMB) {
      type_id *param_types = symb->u.func.param_types;

      expr_list_t curr = call->args;
      int i = 0;
      for (; param_types[i] && curr; i++, curr = curr->next) {
        MORPH_OR_ERROR(evaluate_expr_type(curr->expr), param_types[i]);
      }

      if (param_types[i]) {
        fprintf(stderr, "Error: too few arguments given\n");
      }
      else if (curr) {
        fprintf(stderr, "Error: too many arguments given\n");
      }

      return symb->u.func.ret_type;
    }
    else {
      // Wrong kind of symbol
      fprintf(stderr, "Error: symbol %s is not a function\n", call->id);
    }
  }
  else {
    // Symbol not found
    fprintf(stderr, "Error: symbol %s not found\n", call->id);
  }

  return -1;
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
      // return UNIT_TYPE;
      return -1;
  }

  return -1;
}

static type_id evaluate_unary_type(unary_t unary) {
  type_id inner = evaluate_expr_type(unary->expr);

  switch (unary->op) {
    case NOT_OP:
      return MORPH_OR_ERROR(inner, BOOLEAN_TYPE);
    case BIT_NOT_OP:
      return MORPH_OR_ERROR(inner, INTEGER_TYPE);
    case NEG_OP:
      return MORPH_OR_ERROR(inner, REAL_TYPE);
    case PRE_INC_OP:
      return MORPH_OR_ERROR(inner, INTEGER_TYPE);
    case PRE_DEC_OP:
      return MORPH_OR_ERROR(inner, INTEGER_TYPE);
    case POST_INC_OP:
      return MORPH_OR_ERROR(inner, INTEGER_TYPE);
    case POST_DEC_OP:
      return MORPH_OR_ERROR(inner, INTEGER_TYPE);
  }

  return -1;
}

static type_id evaluate_binary_type(binary_t binary) {
  type_id target;
  type_id target2;

  type_id left = evaluate_expr_type(binary->left);
  type_id right = evaluate_expr_type(binary->right);

  switch (binary->op) {
    case PLUS_OP:
      target = MORPH_OR_ERROR(left, STRING_TYPE, INTEGER_TYPE,
                                      REAL_TYPE);
      return MORPH_OR_ERROR(right, target);

    case MINUS_OP:
      target = MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      return MORPH_OR_ERROR(right, target);

    case MUL_OP:
      target = MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      return MORPH_OR_ERROR(right, target);

    case DIV_OP:
      target = MORPH_OR_ERROR(left, REAL_TYPE, INTEGER_TYPE);
      target2 = MORPH_OR_ERROR(right, REAL_TYPE, INTEGER_TYPE);
      return target == REAL_TYPE || target2 == REAL_TYPE ? REAL_TYPE
                                                         : INTEGER_TYPE;

    case MOD_OP:
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);

    case AND_OP:
      MORPH_OR_ERROR(left, BOOLEAN_TYPE);
      MORPH_OR_ERROR(right, BOOLEAN_TYPE);
      return BOOLEAN_TYPE;

    case OR_OP:
      MORPH_OR_ERROR(left, BOOLEAN_TYPE);
      MORPH_OR_ERROR(right, BOOLEAN_TYPE);
      return BOOLEAN_TYPE;

    case XOR_OP:
      MORPH_OR_ERROR(left, BOOLEAN_TYPE);
      MORPH_OR_ERROR(right, BOOLEAN_TYPE);
      return BOOLEAN_TYPE;

    case BIT_AND_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return INTEGER_TYPE;

    case BIT_OR_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return INTEGER_TYPE;

    case BIT_XOR_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return INTEGER_TYPE;

    case SHIFT_RIGHT_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return INTEGER_TYPE;

    case SHIFT_LEFT_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE);
      return INTEGER_TYPE;

    case EQ_EQ_OP:
      MORPH_OR_ERROR(right, left);
      return BOOLEAN_TYPE;

    case LT_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE, REAL_TYPE);
      return BOOLEAN_TYPE;

    case GT_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE, REAL_TYPE);
      return BOOLEAN_TYPE;

    case NOT_EQ_OP:
      MORPH_OR_ERROR(right, left);
      return BOOLEAN_TYPE;

    case LT_EQ_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE, REAL_TYPE);
      return BOOLEAN_TYPE;

    case GT_EQ_OP:
      MORPH_OR_ERROR(left, INTEGER_TYPE, REAL_TYPE);
      MORPH_OR_ERROR(right, INTEGER_TYPE, REAL_TYPE);
      return BOOLEAN_TYPE;
  }

  return -1;
}

static type_id evaluate_ternary_type(ternary_t ternary) {
  type_id left = evaluate_expr_type(ternary->left);
  type_id middle = evaluate_expr_type(ternary->middle);
  type_id right = evaluate_expr_type(ternary->right);

  switch (ternary->op) {
    case IF_ELSE_OP:
      MORPH_OR_ERROR(left, BOOLEAN_TYPE);
      MORPH_OR_ERROR(right, middle);
      MORPH_OR_ERROR(middle, right);

      return middle;
  }

  return -1;
}

static type_id evaluate_expr_type(expr_t expr) {
  switch (expr->kind) {
    case ID_EX:
      evaluate_id_type(expr->u.id_ex, expr->scope);
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
    default:
      // TODO fail with 'unsupported'
      return -1;
  }
}

static int check_stmt(stmt_t stmt) {
  if (stmt->next) {
    return check_stmt(stmt->next);
  }
  else {
    return 0;
  }
}

static int check_decls(decl_t decl) {
  return 0;
}

static int check_mod(mod_t mod) {

  if (mod->next) {
    return check_mod(mod->next);
  }
  else {
    return 0;
  }
}

int check_types(root_t root, type_node_t *graph) {
  init_types(graph);

  return check_mod(root->mods);
}

