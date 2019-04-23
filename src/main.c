#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "util.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing file argument\n");
    exit(1);
  }
  ll_t tokens = lex(argv[1]);
  while (tokens) {
    token_t t = tokens->val;
    if (t->tok == STRING_VAL)
      printf("\"%s\"\n", t->val);
    else if (t->tok == INT_VAL)
      printf("<%s>\n", t->val);
    else if (t->tok == REAL_VAL)
      printf("$%s$\n", t->val);
    else
      printf("%d:%s\n", t->line_no, t->val);
    tokens = tokens->next;
  }
  printf("\n");
}
