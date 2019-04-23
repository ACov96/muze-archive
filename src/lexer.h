#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

enum token {
            // if statements
            IF, FI, ELIF, ELSE, THEN,
            // case statments
            CASE, ESAC, OF,
            // Looping
            FOR, ROF, LOOP, POOL, BREAK, CONTINUE, CYCLE, FA, AF,
            // Functions
            FUN, NUF,
            // types
            TYPE, EPYT, MU, UM,
            // basic types
            CHAR, STRING, INTEGER, REAL, BOOLEAN, BITSET,
            // composite types
            REC, ARRAY, TUPLE, HASH, SET, LIST,
            // literals
            STRING_VAL, INT_VAL, REAL_VAL,
            TRUE, FALSE,
            // modules
            MOD, DOM, FROM, IMPORT, EXTERN,
            // colons
            COLON, SEMICOLON,
            // comma
            COMMA,
            // parentheses
            LBRACE, RBRACE, LBRACKET, RBRACKET, LPAREN, RPAREN,
            // assignment
            EQ, COLON_EQ, COLON_COLON,
            INC, DEC, PLUS_EQ, MINUS_EQ, MULT_EQ, DIV_EQ,
            // arithmetic
            PLUS, MINUS, MULT, DIV, MODULO,
            // comparison
            EQ_EQ, LT, GT, NOT_EQ, LT_EQ, GT_EQ,
            // boolean
            NOT, AND, OR, XOR,
            // bitwise
            BIT_NOT, BIT_AND, BIT_OR, BIT_XOR,
            // block
            BEGIN, END,
            // accessors
            DOT, TICK,
            // qualifiers
            DOT_DOT, DOT_DOT_DOT, ST, IN,
            // identifiers
            IDENTIFIER
};

struct token_st {
  enum token tok;
  char* val;
  int line_no;
};

typedef struct token_st* token_t;

/* Lex A File
 * file_name - Name of file to generate tokens from
 *
 * Generates a linked list of tokens from the given input file f.
 */
ll_t lex(char* file_name);

#endif

