#ifndef _MORPH_GRAPH_H
#define _MORPH_GRAPH_H

#include "ast.h"

typedef struct type_node_st		*type_node_t;

type_node_t* build_graph(root_t root);
void print_graph(type_node_t* graph);

#endif
