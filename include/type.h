#pragma once

typedef int type_id;

#include "morph_graph.h"
#include "ast.h"

int check_types(root_t root, type_node_t *graph);

#ifndef TYPE_C
// Builtin
extern type_id INTEGER_TYPE; // implemented
extern type_id REAL_TYPE; // implemented
extern type_id BOOLEAN_TYPE; // implemented
extern type_id STRING_TYPE; // implemented
extern type_id UNIT_TYPE;
extern type_id LIST_TYPE;
extern type_id HASHMAP_TYPE;
#endif

