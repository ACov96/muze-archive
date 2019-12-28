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
struct mini_type_st {
  char *name;
  unsigned long morph_length;
  char *morphs[];
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
  long z = (long)(x->members[0]) + (long)(y->members[0]);
  return alloc_int(z);
}

data_t _sub(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(x->members[0]) - (long)(y->members[0]);
  return alloc_int(z);
}

data_t _mul(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(x->members[0]) * (long)(y->members[0]);
  return alloc_int(z);
}

data_t _div(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(x->members[0]) - (long)(y->members[0]);
  return alloc_int(z);
}

data_t _and(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(x->members[0]) && (long)(y->members[0]);
  return alloc_bool(z);
}

data_t _or(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (long)(x->members[0]) || (long)(y->members[0]);
  return alloc_bool(z);
}

data_t _xor(data_t x, data_t y) {
  // TODO: Take type header into account
  if ((long)(x->members[0]) && !((long)(y->members[0]))) return alloc_bool(1);
  else if (!((long)(x->members[0])) && ((long)(y->members[0]))) return alloc_bool(1);
  else return alloc_bool(0);
}

data_t _b_and(data_t x, data_t y) {
  long z = ((long)x->members[0]) & ((long)y->members[0]);
  return alloc_int(z);
}

data_t _b_or(data_t x, data_t y) {
  long z = ((long)x->members[0]) | ((long)y->members[0]);
  return alloc_int(z);
}

data_t _b_xor(data_t x, data_t y) {
  long z = ((long)x->members[0]) ^ ((long)y->members[0]);
  return alloc_int(z);
}

data_t _b_r_shift(data_t x, data_t y) {
  long z = ((long)x->members[0]) >> ((long)y->members[0]);
  return alloc_int(z);
}

data_t _b_l_shift(data_t x, data_t y) {
  long z = ((long)x->members[0]) << ((long)y->members[0]);
  return alloc_int(z);
}

data_t _eq_eq(data_t x, data_t y) {
  long z = ((long)x->members[0]) == ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _lt(data_t x, data_t y) {
  long z = ((long)x->members[0]) < ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _gt(data_t x, data_t y) {
  long z = ((long)x->members[0]) > ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _neq(data_t x, data_t y) {
  long z = ((long)x->members[0]) != ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _lte(data_t x, data_t y) {
  long z = ((long)x->members[0]) >= ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _gte(data_t x, data_t y) {
  long z = ((long)x->members[0]) <= ((long)y->members[0]);
  return alloc_bool(z);
}

data_t _not(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_bool(!z);
}

data_t _b_not(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(~z);
}

data_t _neg(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(-z);
}

data_t _pre_inc(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(++z);
}

data_t _pre_dec(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(--z);
}

data_t _post_inc(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(z++);
}

data_t _post_dec(data_t x) {
  long z = ((long)x->members[0]);
  return alloc_int(z--);
}

data_t __morph__integer_string(data_t in) {
  long old_val = (long)(in->members[0]);
  int digits = 0;
  while (old_val % ((long)pow(10, digits)) < old_val) {
    digits++;
  }
  char *val = malloc(sizeof(char) * digits);
  sprintf(val, "%ld", old_val);
  return alloc_str(val);
}

data_t __morph__string_integer(data_t in) {
  char *str = ((char*)in->members[0]);
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
  long l = (long)in->members[0];
  return alloc_bool(!!l);
}

data_t __morph__boolean_integer(data_t in) {
  long b = (long)in->members[0];
  return alloc_int(b);
}

data_t __morph__boolean_string(data_t in) {
  long b = (long)in->members[0];
  if (b) return alloc_str("true");
  return alloc_str("false");
}

data_t __morph__string_boolean(data_t in) {
  char *str = (char*)in->members[0];
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
    
  }
}

type_descriptor_t __get_data_type_header(data_t *d) {
  return 0;
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
      graph = add_morph(graph, type_name, (*t)->morphs[i], NULL);
    }
  }
  print_graph(graph);
}

void __activate_type(char *type) {
  graph = activate_node(graph, type);
}

void __deactivate_type(char *type) {
  graph = deactivate_node(graph, type);
}

/* METHODS TO BE REMOVED
 *
 * Methods below this point are solely here for debugging purposes. They should eventually be removed.
 */

void print_int(data_t d) {
  long l = (long)d->members[0];
  printf("%ld\n", l);
}

void print_bool(data_t d) {
  long b = (long)d->members[0];
  if (b) {
    printf("true\n");
  } else {
    printf("false\n");
  }
}

void print_real(data_t d) {
  union {
    double r;
    unsigned long l;
  } u;
  u.l = (unsigned long)d->members[0];
  printf("%f\n", u.r);
}

