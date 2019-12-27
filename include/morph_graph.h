#ifndef _MORPH_GRAPH_H
#define _MORPH_GRAPH_H

#include "ast.h"

typedef struct type_node_st		*type_node_t;

type_node_t* build_graph(root_t root);
void print_graph(type_node_t* graph);
char** shortest_path(type_node_t* graph, char* src, char* dest);
type_node_t* activate_node(type_node_t* graph, char* type_name);
type_node_t* deactivate_node(type_node_t* graph, char* type_name);
char** get_type_names(type_node_t* graph);
type_node_t* morph_graph();
type_node_t* add_type(type_node_t* graph, char* type_name);

#endif
