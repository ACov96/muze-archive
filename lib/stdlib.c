/* TODO:
 * - Rework this whole module to use the dynamic typing system. The type header for all of these
 *   functions is just being assumed, but it should be part of the runtime.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
CREATE_TYPE_HEADER(BOOL_HEADER, 0);

void panic(char *msg) {
  fprintf(stderr, "RUNTIME PANIC: %s\n", msg);
  exit(1);
}

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

data_t alloc_bool(long x) {
  long *p = malloc(sizeof(long));
  *p = x;
  return (data_t)(BOOL_HEADER | (unsigned long)p);
}

data_t _add(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) + (*(long*)y);
  return alloc_int(z);
}

data_t _sub(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) - (*(long*)y);
  return alloc_int(z);
}

data_t _mul(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) * (*(long*)y);
  return alloc_int(z);
}

data_t _div(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) / (*(long*)y);
  return alloc_int(z);
}

data_t _and(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) && (*(long*)y);
  return alloc_bool(z);
}

data_t _or(data_t x, data_t y) {
  // TODO: Take type header into account
  long z = (*(long*)x) || (*(long*)y);
  return alloc_bool(z);
}

data_t _xor(data_t x, data_t y) {
  // TODO: Take type header into account
  if ((*(long*)x) && !(*(long*)y)) return alloc_bool(1);
  else if (!(*(long*)x) && (*(long*)y)) return alloc_bool(1);
  else return alloc_bool(0);
}

data_t _b_and(data_t x, data_t y) {
  long z = (*(long*)x) & (*(long*)y);
  return alloc_int(z);
}

data_t _b_or(data_t x, data_t y) {
  long z = (*(long*)x) | (*(long*)y);
  return alloc_int(z);
}

data_t _b_xor(data_t x, data_t y) {
  long z = (*(long*)x) ^ (*(long*)y);
  return alloc_int(z);
}

data_t _b_r_shift(data_t x, data_t y) {
  long z = (*(long*)x) >> (*(long*)y);
  return alloc_int(z);
}

data_t _b_l_shift(data_t x, data_t y) {
  long z = (*(long*)x) << (*(long*)y);
  return alloc_int(z);
}

data_t _eq_eq(data_t x, data_t y) {
  long z = (*(long*)x) == (*(long*)y);
  return alloc_bool(z);
}

data_t _lt(data_t x, data_t y) {
  long z = (*(long*)x) < (*(long*)y);
  return alloc_bool(z);
}

data_t _gt(data_t x, data_t y) {
  long z = (*(long*)x) > (*(long*)y);
  return alloc_bool(z);
}

data_t _neq(data_t x, data_t y) {
  long z = (*(long*)x) != (*(long*)y);
  return alloc_bool(z);
}

data_t _lte(data_t x, data_t y) {
  long z = (*(long*)x) >= (*(long*)y);
  return alloc_bool(z);
}

data_t _gte(data_t x, data_t y) {
  long z = (*(long*)x) <= (*(long*)y);
  return alloc_bool(z);
}

data_t _not(data_t x) {
  long z = (*(long*)x);
  return alloc_bool(!z);
}

data_t _b_not(data_t x) {
  long z = (*(long*)x);
  return alloc_int(~z);
}

data_t _neg(data_t x) {
  long z = (*(long*)x);
  return alloc_int(-z);
}

data_t _pre_inc(data_t x) {
  long z = (*(long*)x);
  return alloc_int(++z);
}

data_t _pre_dec(data_t x) {
  long z = (*(long*)x);
  return alloc_int(--z);
}

data_t _post_inc(data_t x) {
  long z = (*(long*)x);
  return alloc_int(z++);
}

data_t _post_dec(data_t x) {
  long z = (*(long*)x);
  return alloc_int(z--);
}

data_t __morph__integer_string(data_t in) {
  long old_val = *((long*)in);
  int digits = 0;
  while (old_val % ((long)pow(10, digits)) < old_val) {
    digits++;
  }
  char *val = malloc(sizeof(char) * digits);
  sprintf(val, "%ld", *(long*)in);
  return alloc_str(val);
}

data_t __morph__string_integer(data_t in) {
  char *str = ((string_t)in)->str;
  return alloc_int(strtol(str, &str, 10));
}

data_t __morph__real_string(data_t in) {
  return NULL;
}

data_t __morph__string_real(data_t in) {
  return NULL;
}

data_t __morph__real_integer(data_t in) {
  return NULL;
}

data_t __morph__integer_real(data_t in) {
  return NULL;
}

data_t __morph__integer_boolean(data_t in) {
  return NULL;
}

data_t __morph__boolean_integer(data_t in) {
  return NULL;
}

/* METHODS TO BE REMOVED
 *
 * Methods below this point are solely here for debugging purposes. They should eventually be removed.
 */

void print_int(data_t d) {
  long *p = (long *)((~TYPE_MASK) & (unsigned long)d);
  printf("%ld\n", *p);
}

void print_bool(data_t d) {
  long *p = (long *)((~TYPE_MASK) & (unsigned long)d);
  if (*p) {
    printf("true\n");
  } else {
    printf("false\n");
  }
}
