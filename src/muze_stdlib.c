/* TODO:
 * - Rework this whole module to use the dynamic typing system. The type header for all of these
 *   functions is just being assumed, but it should be part of the runtime.
 */
#define UNW_LOCAL_ONLY
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ucontext.h>
#include "muze_stdlib.h"
#include "morph_graph.h"

#define WORD 8
#define TYPE_MASK (~(0xFFFFULL << 48))

typedef struct mini_type_st *mini_type_t;
typedef struct exception_stack_st *exception_stack_t;

struct exception_stack_st {
  ucontext_t context;
  data_t exception;
  exception_stack_t prev;
};

// Helper functions
data_t create_dope_vec(int num_dims, int *dimensions);
long calc_index(data_t idx, data_t arr);

// Helper functions
data_t create_dope_vec(int num_dims, int *dimensions);
long calc_index(data_t idx, data_t arr);

struct mini_morph_st {
  char *dest;
  morph_f morph_fun;
};

struct mini_type_st {
  char *name;
  char *parent_name;
  morph_f parent_to_child_morph;
  morph_f child_to_parent_morph;
  unsigned long morph_length;
  struct mini_morph_st morphs[];
};

extern struct mini_type_st __TYPE_GRAPH;
extern unsigned __TYPE_GRAPH_END;

type_node_t *graph = NULL;
exception_stack_t exception_stack = NULL;
data_t curr_exception = NULL;

void panic(char *msg) {
  fprintf(stderr, "RUNTIME PANIC: %s\n", msg);
  exit(1);
}

void print(data_t d) {
  char* msg = __get_data_member(__morph(d, "string"), 0);
  printf("%s\n", msg);
}

data_t alloc_int(long x) {
  data_t d = __create_new_data(1);
  __set_data_type_header(&d, get_type_index(graph, "integer"));
  __set_data_member(d, (member_t)x, 0);
  return d;
}

data_t alloc_str(char *s) {
  data_t d = __create_new_data(1);
  __set_data_type_header(&d, get_type_index(graph, "string"));
  __set_data_member(d, (member_t)s, 0);
  return d;
}

data_t alloc_bool(long x) {
  data_t d = __create_new_data(1);
  __set_data_type_header(&d, get_type_index(graph, "boolean"));
  __set_data_member(d, (member_t)x, 0);
  return d;
}

/* We pass in the double as an unsigned long because the argument
 * is going to be passed in through %rdi, which requires a non-floating
 * point register to be used. This is ideal though because we would
 * need to convert it anyways.
 */
data_t alloc_real(unsigned long x) {
  data_t d = __create_new_data(1);
  d->members[0] = (member_t)x;
  __set_data_type_header(&d, get_type_index(graph, "real"));
  __set_data_member(d, (member_t)x, 0);
  return d;
}

data_t alloc_array(int n) {
  // TODO: add dope vector with length, upper, and lower bounds.
  // first member points to dope vector
  // figure out how to layout members for multi-dimensional arrays
  // probably column major
  data_t d = __create_new_data(n);
  __set_data_type_header(&d, get_type_index(graph, "array"));
  return d;
}

/* Used for arrays that are declared, but not initialized. 
Sets all members of the given array to 0 */
data_t init_default_array(data_t dims, char *array_type) {
  int num_dims = dims->length;
  long array_size = 1;
  int *dimensions = malloc(sizeof(int) * num_dims);
  int curr_dim = 0;
  // calculate total array size from dimensions
  for (int i = 0; i < num_dims; i++) {
    curr_dim = (long)__get_data_member((__get_data_member(dims, i)), 0);
    if (curr_dim <= 0) panic("Cannot initialize array with dimension size <= 0");
    array_size *= curr_dim;
    dimensions[i] = curr_dim;
  }
  // add 1 to array_size to account for the dope vector in index 0
  array_size++;
  // create the dope vector for the array
  data_t dope_vec = create_dope_vec(num_dims, dimensions);
  data_t d = __create_new_data(array_size);
  __set_data_type_header(&d, get_type_index(graph, array_type));
  // set first member in the array to be the dope vector
  __set_data_member(d, dope_vec, 0);
  if (strcmp(array_type, "array") != 0) {
    // cut off "array of " portion of array type string to get data type
    char *member_type = array_type + 9;
    for (int i = 1; i < array_size; i++)
      __set_data_member(d, __morph(alloc_int(0), member_type), i);
  } else {
    for (int i = 1; i < array_size; i++)
      __set_data_member(d, alloc_int(0), i);
  }

  return d;
}

data_t create_dope_vec(int num_dims, int *dimensions) {
  // allocate 2 members (upper and lower bounds) for each dimensions
  data_t d = alloc_array(num_dims*2);

  for (int i = 0; i < num_dims*2; i+=2) {
    // set lower bound
     __set_data_member(d, alloc_int(0), i);
     // set upper bound
     __set_data_member(d, alloc_int(dimensions[i/2]), i+1);
  }
  return d;
}

data_t _add(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) + (long)(__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _sub(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) - (long)(__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _mul(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) * (long)(__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _div(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) - (long)(__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _and(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) && (long)(__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _or(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(__get_data_member(x, 0)) || (long)(__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _xor(data_t x, data_t y) {
  // TODO: Take type header into account
  if ((long)(__get_data_member(x, 0)) && !((long)(__get_data_member(y, 0)))) return alloc_bool(1);
  else if (!((long)(__get_data_member(x, 0))) && ((long)(__get_data_member(y, 0)))) return alloc_bool(1);
  else return alloc_bool(0);
}

data_t _b_and(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) & ((long)__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _b_or(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) | ((long)__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _b_xor(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) ^ ((long)__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _b_r_shift(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) >> ((long)__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _b_l_shift(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) << ((long)__get_data_member(y, 0));
  return alloc_int(z);
}

data_t _eq_eq(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) == ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _lt(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) < ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _gt(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) > ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _neq(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) != ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _lte(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) >= ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _gte(data_t x, data_t y) {
  long z = ((long)__get_data_member(x, 0)) <= ((long)__get_data_member(y, 0));
  return alloc_bool(z);
}

data_t _not(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_bool(!z);
}

data_t _b_not(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(~z);
}

data_t _neg(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(-z);
}

data_t _pre_inc(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(++z);
}

data_t _pre_dec(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(--z);
}

data_t _post_inc(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(z++);
}

data_t _post_dec(data_t x) {
  long z = ((long)__get_data_member(x, 0));
  return alloc_int(z--);
}

data_t __morph__integer_string(data_t in) {
  long old_val = (long)__get_data_member(in, 0);
  int digits = 0;
  while (old_val % ((long)pow(10, digits)) < old_val) {
    digits++;
  }
  char *val = malloc(sizeof(char) * digits);
  sprintf(val, "%ld", old_val);
  return alloc_str(val);
}

data_t __morph__string_integer(data_t in) {
  char *str = __get_data_member(in, 0);
  return alloc_int(strtol(str, &str, 10));
}

/* data_t __morph__real_string(data_t in) { */
/*   char *str = malloc(64); // TODO: Make this dynamic */
/*   sprintf(str, "%f", (double)in->members[0]); */
/*   return alloc_str(str); */
/* } */

/* data_t __morph__string_real(data_t in) { */
/*   char *str = ((char*)in->members[0]); */
/*   return alloc_real(atof(str)); */
/* } */

/* data_t __morph__real_integer(data_t in) { */
/*   long l = (long)((double)in->members[0]); */
/*   return alloc_int(l); */
/* } */

data_t __morph__integer_real(data_t in) {
  double d = (double)((unsigned long)__get_data_member(in, 0));
  return alloc_real(d);
}

data_t __morph__integer_boolean(data_t in) {
  long l = (long)__get_data_member(in, 0);
  return alloc_bool(!!l);
}

data_t __morph__boolean_integer(data_t in) {
  long b = (long)__get_data_member(in, 0);
  return alloc_int(b);
}

data_t __morph__boolean_string(data_t in) {
  long b = (long)__get_data_member(in, 0);
  if (b) return alloc_str("true");
  return alloc_str("false");
}

data_t __morph__string_boolean(data_t in) {
  char *str = (char*)__get_data_member(in, 0);
  if (strcmp(str, "true") == 0) return alloc_bool(1);
  else if (strcmp(str, "false") == 0) return alloc_bool(0);
  panic("Unknown boolean string value");
  return NULL;
}

data_t __create_new_data(unsigned long size) {
  if (size < 1) panic("Cannot allocate data structure with size less than 1 member.");
  data_t d = malloc(sizeof(struct data_st) + sizeof(member_t) * size);
  d->length = size;
  for (int i = 0; i < size; i++)
    d->members[i] = NULL;
  return d;
}

member_t __get_data_member(data_t d, int idx) {
  //char *type_name = get_type_name(graph, __get_data_type_header(d));
  //printf("d type = %s, index = %d\n", type_name, idx);
  data_t d_masked = (data_t)((unsigned long)(d) & TYPE_MASK);
  if (d_masked->length <= idx) panic("Data index out of bounds in get");
  return d_masked->members[idx];
}

void __set_data_member(data_t d, member_t c, int idx) {
  data_t d_masked = (data_t)((unsigned long)(d) & TYPE_MASK);
  if (d_masked->length <= idx) panic("Data index out of bounds in set");
  d_masked->members[idx] = c;
}

void __set_data_type_header(data_t *d, type_descriptor_t td) {
  data_t d_masked = (data_t)((unsigned long)(*d) & TYPE_MASK);
  if (td < 0xFFF) {
    d_masked->type_overflow = 0;
    *d = (data_t)((unsigned long)d_masked | (td << 48));
  } else {
    d_masked->type_overflow = td;
    *d = (data_t)((unsigned long)d_masked | (0xFFFUL << 48));
  }
}

type_descriptor_t __get_data_type_header(data_t d) {
  return (type_descriptor_t)(((unsigned long)d & (0xFFFUL << 48)) >> 48);
}

void init_type_graph() {
  graph = morph_graph();
  for (mini_type_t *t = (mini_type_t*)&__TYPE_GRAPH;
       (unsigned long)t < (unsigned long)(&__TYPE_GRAPH_END);
       t++) {
    char *type_name = (*t)->name;
    graph = add_type(graph, type_name);
  }
  for (mini_type_t *t = (mini_type_t*)&__TYPE_GRAPH;
       (unsigned long)t < (unsigned long)(&__TYPE_GRAPH_END);
       t++) {
    char *type_name = (*t)->name;
    unsigned long num_morphs = (*t)->morph_length;
    for (unsigned long i = 0; i < num_morphs; i++) {
      graph = add_morph(graph, type_name, (*t)->morphs[i].dest, (*t)->morphs[i].morph_fun);
    }
  }

  for (mini_type_t *t = (mini_type_t*)&__TYPE_GRAPH;
       (unsigned long)t < (unsigned long)(&__TYPE_GRAPH_END);
       t++) {
    char *type_name = (*t)->name;
    char *parent_name = (*t)->parent_name;
    graph = add_morph(graph, parent_name, type_name, (*t)->parent_to_child_morph);
    graph = add_morph(graph, type_name, parent_name, (*t)->child_to_parent_morph);
  }

  // Add standard morphs and activate them
  set_morph(graph, "integer", "string", &__morph__integer_string);
  set_morph(graph, "integer", "boolean", &__morph__integer_boolean);
  set_morph(graph, "string", "integer", &__morph__string_integer);
  set_morph(graph, "string", "boolean", &__morph__string_boolean);
  set_morph(graph, "boolean", "string", &__morph__boolean_string);
  set_morph(graph, "boolean", "integer", &__morph__boolean_integer);
  set_morph(graph, "integer", "real", &__morph__integer_real);
  // set_morph(graph, "real", "integer", &__morph__real_integer);
}

void __activate_type(char *type) {
  graph = activate_node(graph, type);
}

void __deactivate_type(char *type) {
  graph = deactivate_node(graph, type);
}

void __add_type(char *type) {
  graph = add_type(graph, type);
}

data_t __morph(data_t d, char *target) {

  char *curr_type_name = get_type_name(graph, __get_data_type_header(d));
  //printf("current_type: %s, target: %s\n", curr_type_name, target);
  if (strcmp(target, "") == 0)
    panic("No target data type provided");
  if (strcmp(curr_type_name, target) == 0)
    return d;
  char **path = shortest_path(graph, curr_type_name, target);
  if (path == NULL)
    panic("Unable to perform morph");
  data_t ret = d;
  for (int i = 0; path[i+1]; i++) {
    morph_f morph_fun = get_morph(graph, path[i], path[i+1]);
    if (morph_fun == NULL)
      panic("Unable to load morph function");
    ret = morph_fun(ret);
  }
  return ret;
}

void __assign_simple(data_t src, data_t dest) {
  int dest_type_header = __get_data_type_header(dest);
  data_t src_matching_type
    = __morph(src, get_type_name(graph, dest_type_header));

  /* TODO:
   * This only works for primitive types, so extend this to work for 
   * congolmerate types.
   */
  /*
  if (dest_type_header != get_type_index(graph, "integer")
      && dest_type_header != get_type_index(graph, "real")
      && dest_type_header != get_type_index(graph, "string")
      && dest_type_header != get_type_index(graph, "boolean"))
    panic("Assigning non-primitive type");
  */
  __set_data_member(dest, __get_data_member(src_matching_type, 0), 0);
}

ucontext_t *_init_try(void) {
  exception_stack_t new_exception = malloc(sizeof(struct exception_stack_st));
  new_exception->prev = exception_stack;
  new_exception->exception = NULL;
  exception_stack = new_exception;
  return &exception_stack->context;
}

bool _check_exception(void) {
  return exception_stack != NULL && exception_stack->exception != NULL;
}

data_t _get_exception(void) {
  return exception_stack != NULL ? exception_stack->exception : NULL;
}

void _clear_try(void) {
  exception_stack_t curr = exception_stack;
  exception_stack = curr->prev;
  free(curr);
}

void _throw_exception(data_t exception) {
  exception_stack->exception = exception;
  setcontext(&exception_stack->context);
}

data_t __identity_helper(data_t d, char *type_name) {
  type_descriptor_t td = get_type_index(graph, type_name);
  __set_data_type_header(&d, td);
  return d;
}
  
/* If dest is a member of an untyped array, then set the data member to src. 
   Else call __assign_simple()*/
void __assign_array_member(data_t src, data_t dest, data_t arr) {
  long index = calc_index(idx, arr);
  char *array_type = get_type_name(graph, __get_data_type_header(arr));
  data_t dest = __get_data_member(arr, index);
  if (strcmp(array_type, "array") == 0)
    __set_data_member(dest, __get_data_member(src, 0), 0);
  else
    __assign_simple(src, dest);
}

/* calculates the index in the array in memory */
long calc_index(data_t idx, data_t arr) {
  //char *type_name = get_type_name(graph, __get_data_type_header(arr));
  //printf("arr type = %s\n", type_name);

  if (idx->length == 1) {
    return (long)__get_data_member(__get_data_member(idx, 0), 0) + 1;
  }
  int l = idx->length;
  //printf("index length is %d\n", l);
  data_t d_vec = __get_data_member(arr, 0);
  long curr_dim = (long)__get_data_member(__get_data_member(idx, 0), 0);
  long curr_bound = (long)__get_data_member(__get_data_member(d_vec, 1), 0);;
  long final_index = curr_dim + 1;
  int d = 3;
  for (int i = 1; i < l; i++) {
    curr_dim = (long)__get_data_member(__get_data_member(idx, i), 0);
    curr_bound = (long)__get_data_member(__get_data_member(d_vec, d), 0);
    final_index += curr_dim * pow(curr_bound, i);
    d += 2;
  }
  //printf("final_index = %ld\n", final_index);
  return final_index;
}
