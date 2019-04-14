#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

enum token {
            IF,
            FI
};

struct token_st {
  enum token tok;
  char* val;
};

typedef struct token_st* token_t;

/* Lex A File
 * file_name - Name of file to generate tokens from
 *
 * Generates a linked list of tokens from the given input file f.
 */
ll_t lex(char* file_name);

#endif
