#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

enum token {
            IF, FI, ELIF, ELSE, THEN,
            FOR, ROF, LOOP, POOL, BREAK, CONTINUE, CYCLE,
            CASE, ESAC, OF,
            FUN, NUF,
            TYPE, EPYT, MU, UM,
            MOD, DOM,
            SEMICOLON, LBRACE, RBRACE, LBRACKET, RBRACKET, LPAREN, RPAREN,
            INC, DEC, PLUS_EQ, PLUS, MINUS, MINUS_EQ, MULT, MULT_EQ, DIV, DIV_EQ,
            CHAR, STRING, INTEGER, REAL, BOOLEAN, STRING_VAL, INT,
            REC, ARRAY, TUPLE, HASH, SET, LIST,
            BEGIN, END,
            BITSET,
            TRUE, FALSE,
            TICK,
            FROM, IMPORT,
            MODULO, EXTERN,
            ST, IN,
            IDENTIFIER
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
