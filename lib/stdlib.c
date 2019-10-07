/* TODO:
 * - Rework this whole module to use the dynamic typing system. The type header for all of these
 *   functions is just being assumed, but it should be part of the runtime.
 */
#include <stdlib.h>
#include <stdio.h>

#define CREATE_TYPE_HEADER(T, V) const unsigned long T = (unsigned long)(V) << 48;

typedef void* data_t;

CREATE_TYPE_HEADER(TYPE_MASK, 0xFFFF);
CREATE_TYPE_HEADER(INT_HEADER, 1);

void print(data_t d) {
  char* msg = (char*)((unsigned long)d & ~TYPE_MASK);
  printf("%s\n", msg);
}

data_t alloc_int(long x) {
  long *p = malloc(sizeof(long));
  *p = x;
  return (data_t) (INT_HEADER | (unsigned long)p);
}

/* METHODS TO BE REMOVED
 *
 * Methods below this point are solely here for debugging purposes. They should eventually be removed.
 */

void _print_int(data_t d) {
  long *p = (long *)((~TYPE_MASK) & (unsigned long)d);
  printf("%ld\n", *p);
}
