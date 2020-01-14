/* TODO:
 * - Rework this whole module to use the dynamic typing system. The type header for all of these
 *   functions is just being assumed, but it should be part of the runtime.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "muze_stdlib.h"
#include "morph_graph.h"

#define WORD 8
#define TYPE_MASK (~(0xFFFFULL << 48))

typedef struct mini_type_st *mini_type_t;

struct mini_morph_st {
  char *dest;
  morph_f morph_fun;
};

struct mini_type_st {
  char *name;
  unsigned long morph_length;
  struct mini_morph_st morphs[];
};


extern struct mini_type_st __TYPE_GRAPH;
extern unsigned __TYPE_GRAPH_END;

type_node_t *graph = NULL;

void panic(char *msg) {
  fprintf(stderr, "RUNTIME PANIC: %s\n", msg);
  exit(1);
}

void print(data_t d) {
  char* msg = __get_data_member(d, 0);
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

/* data_t __morph__integer_real(data_t in) { */
/*   double d = (double)((long)in->members[0]); */
/*   return alloc_real(d); */
/* } */

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

  // Add standard morphs and activate them
  set_morph(graph, "integer", "string", &__morph__integer_string);
  set_morph(graph, "integer", "boolean", &__morph__integer_boolean);
  set_morph(graph, "string", "integer", &__morph__string_integer);
  set_morph(graph, "string", "boolean", &__morph__string_boolean);
  set_morph(graph, "boolean", "string", &__morph__boolean_string);
  set_morph(graph, "boolean", "integer", &__morph__boolean_integer);
}

void __activate_type(char *type) {
  graph = activate_node(graph, type);
}

void __deactivate_type(char *type) {
  graph = deactivate_node(graph, type);
}

data_t __morph(data_t d, char *target) {
  char *curr_type_name = get_type_name(graph, __get_data_type_header(d));
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
  if (dest_type_header != get_type_index(graph, "integer")
      && dest_type_header != get_type_index(graph, "real")
      && dest_type_header != get_type_index(graph, "string")
      && dest_type_header != get_type_index(graph, "boolean"))
    panic("Assigning non-primitive type");
  __set_data_member(dest, __get_data_member(src_matching_type, 0), 0);
}
