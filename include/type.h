#pragma once

typedef unsigned long type_id;

#include "ast.h"

type_id new_type_id();

type_id evaluate_expr_type(expr_t expr);

// Builtin
extern const type_id STRING_TYPE;
extern const type_id BOOLEAN_TYPE;
extern const type_id INTEGER_TYPE;
extern const type_id REAL_TYPE;
extern const type_id UNIT_TYPE;
extern const type_id LIST_TYPE;
extern const type_id HASHMAP_TYPE;

