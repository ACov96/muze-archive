#ifndef _MORPH_GRAPH_H
#define _MORPH_GRAPH_H

#include "ast.h"
#include "muze_stdlib.h"

typedef struct type_node_st		*type_node_t;

type_node_t* build_graph(root_t root);
void print_graph(type_node_t* graph);
char** shortest_path(type_node_t* graph, char* src, char* dest);
type_node_t* activate_node(type_node_t* graph, char* type_name);
type_node_t* deactivate_node(type_node_t* graph, char* type_name);
char** get_type_names(type_node_t* graph);
type_node_t* morph_graph();
type_node_t* add_type(type_node_t* graph, char* type_name);
type_node_t* add_morph(type_node_t* graph, char* base_type, char* morph_type, morph_f morph_fun);
morph_f get_morph(type_node_t *graph, char *base_type, char *morph_type);
void set_morph(type_node_t *graph, char *base_type, char *morph_type, morph_f morph_fun);
int get_type_index(type_node_t* graph, char* name);
char* get_type_name(type_node_t* graph, int index);

#endif
