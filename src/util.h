#ifndef _UTIL_H
#define _UTIL_H

struct ll_st {
  void *val;
  struct ll_st *next;
};

typedef struct ll_st* ll_t;

/* New Linked List
 *
 * Create a new linked list
 */
ll_t ll_new();

/* Append to Linked List
 * l - Linked list to append data to
 * data - Data to append
 *
 * Append data to the end of a linked list
 */
void ll_append(ll_t l, void *data);

#endif
