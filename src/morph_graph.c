#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morph_graph.h"
#include "ast.h"

#define NUM_PRIMITIVES 4

struct type_node_st {
  char* name;
  int index;
	int active; //inactive = 0, active = 1
  type_node_t next;
};

/* Prototypes */
type_node_t* build_morph_graph(root_t root);
type_node_t* morph_graph();
void print_graph(type_node_t* graph);
type_node_t type_node(char* name, int index); 
type_node_t* add_type(type_node_t* graph, char* type_name);
type_node_t* add_morph(type_node_t* graph, char* base_type, char* morph_type);
int get_type_index(type_node_t* graph, char* name);
type_node_t* activate_node(type_node_t* graph, char* type_name);
type_node_t* deactivate_node(type_node_t* graph, char* type_name);
char** shortest_path(type_node_t* graph, char* src, char* dest);
int* bfs(type_node_t* graph, int src, int dest);

/* Gloabl variables */ 
int next_index = 0;											// next available index in the morph graph
int num_types = NUM_PRIMITIVES;					// number of types in the graph
char* primitive_types[] = {"integer", "real", "string", "boolean"};


/*Function called from main to build morph graph*/
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
	node->active = 1;
  node->next = NULL;
  return node;
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


/* Activate a node in the graph when a type comes into scope */
type_node_t* activate_node(type_node_t* graph, char* type_name) {
	int i = get_type_index(graph, type_name);
	graph[i]->active = 1;
	return 0;
}


/* Deactivate a node in the graph when a type goes out of scope */
type_node_t* deactivate_node(type_node_t* graph, char* type_name) {
	int i = get_type_index(graph, type_name);
	graph[i]->active = 0;
	return 0;
}


/* shortest path is printing in reverse order right now, but it's working */
char** shortest_path(type_node_t* graph, char* src, char* dest) {
	
	int* path;
	int s = get_type_index(graph, src);
	int d = get_type_index(graph, dest);
	
	path = bfs(graph, s, d);

	if (path == NULL)
		return NULL;
	
	int path_length = sizeof(path)/sizeof(path[0]);

	char** final_path = malloc(sizeof(char*) * path_length);

	int i = 1;
	final_path[0] = graph[d]->name;
	while (path[d] != -1) {
		final_path[i] = graph[path[d]]->name;
		d = path[d];
		i++;
	}
	return final_path;
}


int* bfs(type_node_t* graph, int src, int dest) {	
	//int len = sizeof(graph)/sizeof(graph[0]);
	int v = 0;
	while (graph[v]) {
		v++;
	}

	int* q = malloc(sizeof(int) * v * v);
	int curr = 0;
	int p = 1;
	int dist[v];
	int* pred = malloc(sizeof(int) * v);
	int visited[v];

	for (int i = 0; i < v*v; i++){
		q[i] = -1;
	}
	
	for (int i = 0; i < v; i++) {
		visited[i] = 0;
		dist[i] = 0;
		pred[i] = -1;
	}

	visited[src] = 1;
	q[curr] = src;

	while (q[curr] != -1) {
		int c = q[curr];
		curr++;
		type_node_t n = graph[c];
		for (; n; n = n->next) {
			int index = n->index;
			if (visited[index] == 0) {
				visited[index] = 1;
				dist[index] = dist[c] + 1;
				pred[index] = c;

				if (index == dest) {
					return pred;
				}
				
				q[p] = index;
				p++;
			}
		}
	}
	return NULL;
}

