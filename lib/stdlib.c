/* TODO:
 * - Rework this whole module to use the dynamic typing system. The type header for all of these
 *   functions is just being assumed, but it should be part of the runtime.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CREATE_TYPE_HEADER(T, V) const unsigned long T = (unsigned long)(V) << 48;

struct string_st {
  long length;
  char *str;
};

typedef void* data_t;
typedef struct string_st *string_t;

CREATE_TYPE_HEADER(TYPE_MASK, 0xFFFF);
CREATE_TYPE_HEADER(STR_HEADER, 0);
CREATE_TYPE_HEADER(INT_HEADER, 0);

void print(data_t d) {
  char* msg = ((string_t)d)->str;
  printf("%s\n", msg);
}

data_t alloc_int(long x) {
  long *p = malloc(sizeof(long));
  *p = x;
  return (data_t) (INT_HEADER | (unsigned long)p);
}

data_t alloc_str(char *s) {
  string_t heap_str = malloc(sizeof(struct string_st));
  heap_str->length = strlen(s);
  heap_str->str = malloc(heap_str->length + 1);
  strcpy(heap_str->str, s);
  return (data_t) (STR_HEADER | (unsigned long)heap_str);
}

/* METHODS TO BE REMOVED
 *
 * Methods below this point are solely here for debugging purposes. They should eventually be removed.
 */

void print_int(data_t d) {
  long *p = (long *)((~TYPE_MASK) & (unsigned long)d);
  printf("%ld\n", *p);
}
