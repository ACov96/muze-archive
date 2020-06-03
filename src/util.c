#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include "util.h"
#include "limits.h"

FILE *LOG_FILE = NULL;

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

int ll_length(ll_t ll) {
  int length = 0;
  for (ll_t l = ll; l; l = l->next)
    length++;
  return length;
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
  struct err_st {
    char *file;
    int  line;
    int  col;
    char *title;
    char *msg;
  } error;
  err_queue_t next;
};

static err_queue_t errors;

void append_error(char *file, int line, int col, char *title, char *msg, ...) {
  va_list args;
  err_queue_t *curr;

  // get to the end of the list
  for (curr = &errors; *curr; curr = &((*curr)->next));

  *curr = malloc(sizeof(struct err_queue_st));
  (*curr)->error.file = file;
  (*curr)->error.line = line;
  (*curr)->error.col = col;
  (*curr)->error.title = title;
  (*curr)->error.msg = malloc(MAX_ERROR_SIZE);
  (*curr)->next = NULL;

  va_start(args, msg);
  vsnprintf((*curr)->error.msg, MAX_ERROR_SIZE, msg, args);
  va_end(args);
}

#define ANSI_ESC 0x1b

void print_errors() {
  err_queue_t curr;

  fprintf(stderr, "%c[1mErrors%c[0m\n", ANSI_ESC, ANSI_ESC);

  for (curr = errors; curr; curr = curr->next) {
    fprintf(stderr, "%c[1mLine %d, Col %d%c[0m in %s\n",
            ANSI_ESC, curr->error.line, curr->error.col, ANSI_ESC,
            curr->error.file);
    fprintf(stderr, "    %c[1m%s%c[0m\n",
            ANSI_ESC, curr->error.title, ANSI_ESC);
    fprintf(stderr, "    %s\n", curr->error.msg);
  }
}

int had_errors() {
  return errors != NULL;
}

/* TODO: Wow, there has to be a better way to concatenate two strings, but
 *       I'm far too tired to do this cleanly.
 */
char *concat(char* s1, char* s2) {
  // Allocate the total number of characters of each string, plus 1 extra byte for null terminator
  int s1_length = strlen(s1);
  int s2_length = strlen(s2);
  char *res = calloc(s1_length + s2_length + 1, sizeof(char));
  for (int i = 0; i < s1_length; i++)
    res[i] = s1[i];
  for (int i = strlen(s1); i < (s1_length + s2_length); i++)
    res[i] = s2[i - s1_length];
  return res;
}

char *itoa(unsigned int x) {
  char *res = malloc(16);
  sprintf(res, "%d", x);
  return res;
}

char *remove_empty_lines(char *s) {
  char *new_s = calloc(strlen(s) + 1, sizeof(char));
  int new_s_idx = 0;
  for (int i = 0; i < strlen(s); i++) {
    if (i > 0) {
      if (s[i] == '\n' && s[i-1] == '\n') {
        continue;
      } else {
        new_s[new_s_idx] = s[i];
        new_s_idx++;
      }
    } else {
      new_s[new_s_idx] = s[i];
      new_s_idx++;
    }
  }
  return concat(new_s, "\n");
}

unsigned long f_to_int(char *d) {
  union {
    double x;
    unsigned long l;
  } u;
  u.x = atof(d);
  return u.l;
}

char *find_linker_script() {
  return concat(PREFIX, "/share/muze/muze.ld");
}

unsigned long count_char_occurences(char *s, char c) {
  unsigned long count = 0;
  for (unsigned long i = 0; i < strlen(s); i++)
    if (s[i] == c)
      count++;
  return count;
}
