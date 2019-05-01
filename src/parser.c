#include <stdio.h>


#include "ast.h"
#include "lexer.h"


ll_t tokens = NULL; // TODO: should be set in main

char token_names[][12] = {
  "IF", "END OF FILE"
};

enum token current_token;

enum token next_token() {
  if (tokens) {
    enum token t = (enum token)tokens->val;
    tokens = tokens->next;
    return t;
  }
  return LEXEOF;
}
/* Check that current_token matches argument
  and get next token. */
void match(enum token tok) {
  if (current_token == tok) {
    current_token = next_token();
  } else {
    // what the fuck:
    fprintf(stderr, "%u: expected token %d: %s\n",
        ((token_t)tokens->val)->line_no, tok, token_names[tok]);
  }
}

