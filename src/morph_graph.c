#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morph_graph.h"
#include "ast.h"

#define NUM_PRIMITIVES 4

struct type_node_st {
	char* name;
	int index;
	type_node_t next;
};

/* Prototypes */
type_node_t type_node(char* name, int index); 
type_node_t* add_type(type_node_t* graph, char* type_name);
type_node_t* add_morph(type_node_t* graph, char* base_type, char* morph_type);
type_node_t* morph_graph();
int get_type_index(type_node_t* graph, char* name);
void print_graph(type_node_t* graph);

/* Gloabl variables */ 
int next_index = 0;											// next available index in the morph graph
int num_types = NUM_PRIMITIVES;					// number of types in the graph
char* primitive_types[] = {"integer", "real", "string", "boolean"};
char* int_morphs[] = {"real", "string", "boolean"};


/* function for printing out the graph. 
Should have a FILE* out added at some point soon
and printing could defintely be prettier, but good
enough for testing at the moment*/
void print_graph(type_node_t* graph){
	printf("Morph Graph:\n");
	int i = 0;
	while (graph[i]){
		printf("[%d | %s]", i, graph[i]->name);
		if (graph[i]->next){
			type_node_t curr = graph[i]->next;
			for (; curr; curr= curr->next){
				printf(" -> [%d | %s]", curr->index, curr->name);
			}
		}
		printf("\n");
		i++;
	}
}

/* type_node constructor */
type_node_t type_node(char* name, int index) {
	type_node_t node = malloc(sizeof(struct type_node_st));
	node->name = name;
	node->index = index;
	node->next = NULL;
	return node;
}


/* returns the index of the given type name from the given graph */
int get_type_index(type_node_t* graph, char* type_name) {
	int i = 0;
	int type_index = -1;
	while (graph[i]){
		if (strcmp(graph[i]->name, type_name) == 0){
			type_index = graph[i]->index;
			break;
		}
		i++;
	}
	return type_index;
}


/* Add a new type to the given graph */
type_node_t* add_type(type_node_t* graph, char* type_name) {	 
	type_node_t node = type_node(type_name, next_index); 
	graph[node->index] = node;
	next_index++;
	return graph;
}


/* Add a morph to the given type/graph */
type_node_t* add_morph(type_node_t* graph, char* base_type, char* morph_type) {
	int type_index = get_type_index(graph, base_type);
	type_node_t curr = graph[type_index];
	type_node_t morph = type_node(morph_type, get_type_index(graph, morph_type));
	
	for (; curr->next != NULL; curr = curr->next);
	curr->next = morph;
	return graph;
}


/* Morph graph constructor. Returns morph graph with primitives types and morphs added */
type_node_t* morph_graph() {
	type_node_t* graph = malloc(sizeof(struct type_node_st)*NUM_PRIMITIVES);

	// Add primitive types to graph
	for (int i = 0; i < NUM_PRIMITIVES; i++){
		graph = add_type(graph, primitive_types[i]);
	}

	//add primitive morphs
	graph = add_morph(graph, "integer", "real");
	graph = add_morph(graph, "integer", "string");
	graph = add_morph(graph, "integer", "boolean");
	graph = add_morph(graph, "real", "integer");
	graph = add_morph(graph, "real", "string");
	graph = add_morph(graph, "string", "integer");
	graph = add_morph(graph, "string", "real");
	graph = add_morph(graph, "string", "boolean");
	graph = add_morph(graph, "boolean", "string");
	graph = add_morph(graph, "boolean", "integer");

	return graph;
}

type_node_t* build_graph(root_t root) {
	// allocate space for array
	type_node_t* graph = morph_graph();
	type_decl_t type_decl = root->mods->decl->types;
	for (; type_decl; type_decl = type_decl->next) {
		if (next_index >= num_types)
			// If the graph runs out of room double the size
			graph = (type_node_t*) realloc(graph, sizeof(struct type_node_st)*num_types*2);
		graph = add_type(graph, type_decl->name);
	}
	return graph;
}


