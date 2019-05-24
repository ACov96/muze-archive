#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "util.h"
#include "ast.h"
#include "print_tree.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing file argument\n");
    exit(1);
  }
  // lex
  ll_t tokens = lex(argv[1]);
  ll_t toks = tokens;
  
  // print lexed tokens
  while (toks) {
    token_t t = toks->val;
    printf("%s : ", token_names[t->tok].raw);
    if (t->tok == STRING_VAL)
      printf("\"%s\"\n", t->val);
    else if (t->tok == INT_VAL)
      printf("<%s>\n", t->val);
    else if (t->tok == REAL_VAL)
      printf("$%s$\n", t->val);
    else
      printf("%d:%s\n", t->line_no, t->val);
    toks = toks->next;
  }  
  printf("\n");
  
  // parse
  root_t root;
  root = parse(tokens);
  if (root) {
    print_tree(stdout, root);
  }

  if (had_errors()) {
    print_errors();
  }
}
