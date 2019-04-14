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
}
