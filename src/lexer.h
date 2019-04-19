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
            COLON, SEMICOLON, LBRACE, RBRACE, LBRACKET, RBRACKET, LPAREN, RPAREN,
            INC, DEC, PLUS_EQ, PLUS, MINUS, MINUS_EQ, MULT, MULT_EQ, DIV, DIV_EQ,
            EQ,
            EQ_EQ, LT, GT, NOT_EQUAL, LT_EQUAL, GT_EQUAL, // comparison
            NOT, AND, OR, XOR, // boolean
            BIT_NOT, BIT_AND, BIT_OR, BIT_XOR, // bitwise
            CHAR, STRING, INTEGER, REAL, BOOLEAN,
            CHAR_VAL, STRING_VAL, INT_VAL, REAL_VAL,
            REC, ARRAY, TUPLE, HASH, SET, LIST,
            BEGIN, END,
            BITSET,
            TRUE, FALSE,
            DOT, TICK,
            DOTDOT,
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

