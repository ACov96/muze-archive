#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
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

void write_log(char *msg, ...) {
  va_list args;

  va_start(args, msg);
  vfprintf(stdout, msg, args);
  va_end(args);
  fprintf(stdout, "\n");
}

typedef struct err_queue_st *err_queue_t;

struct err_queue_st {
  char *msg;
  err_queue_t next;
};

static err_queue_t errors;

void append_error(char *msg, ...) {
  va_list args;
  err_queue_t *curr;

  // get to the end of the list
  for (curr = &errors; *curr; curr = &((*curr)->next));

  *curr = malloc(sizeof(struct err_queue_st));
  (*curr)->msg = malloc(BUFSIZ);
  snprintf((*curr)->msg, BUFSIZ, msg, args);
  (*curr)->next = NULL;
}

void print_errors() {
  err_queue_t curr;

  for (curr = errors; curr; curr = curr->next) {
    fprintf(stderr, "%s\n", curr->msg);
  }
}

