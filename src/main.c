#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "lexer.h"
#include "util.h"
#include "ast.h"
#include "print_tree.h"

int main(int argc, char* argv[]) {
  /*
  int opt;
  struct option long_opts[] = {
    { "--help",   no_argument,       NULL, 'h' },
    { "--output", required_argument, NULL, 'o' }
  };

  while (1) {
    opt = getopt_long(argc, argv, "", long_opts, NULL);
    //TODO
  }
  */
  // lex
  ll_t tokens = lex(argv[1]);
  ll_t toks = tokens;
  
  // print lexed tokens
  while (toks) {
    token_t t = toks->val;
    printf("(%d,%d) %s : ", t->line_no, t->col_no, token_names[t->tok].raw);
    if (t->tok == STRING_VAL)
      printf("\"%s\"\n", t->val);
    else if (t->tok == INT_VAL)
      printf("<%s>\n", t->val);
    else if (t->tok == REAL_VAL)
      printf("$%s$\n", t->val);
    else
      printf("%s\n", t->val);
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
