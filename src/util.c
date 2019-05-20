#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"

ll_t ll_new() {
  ll_t ll = malloc(sizeof(struct ll_st));
  return ll;
}

void ll_append(ll_t l, void *data) {
  ll_t curr;
  assert(l != NULL);
  if (l->val == NULL) {
    l->val = data;
    return;
  }
  for (curr = l; curr->next != NULL; curr = curr->next);
  curr->next = malloc(sizeof(struct ll_st));
  curr->next->val = data;
}

void error_and_exit(char* message, int line_no) {
  fprintf(stderr, "%d:%s\n", line_no, message);
  exit(1);
}

