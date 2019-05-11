#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

// Don't read just go on and trust it works
#define GEN_ENUM(ident) ident,
#define GEN_STR(ident) #ident,

#define FOREACH_TOKEN(MAC) \
    MAC(IF) MAC(FI) MAC(ELIF) MAC(ELSE) MAC(THEN)  \
    MAC(CASE) MAC(ESAC) MAC(OF)  \
    MAC(FOR) MAC(ROF) MAC(LOOP) MAC(POOL) MAC(BREAK) MAC(CONTINUE) MAC(CYCLE) MAC(FA) MAC(AF)  \
    MAC(FUN) MAC(NUF)  \
    MAC(TYPE) MAC(EPYT) MAC(MU) MAC(UM)  \
    MAC(CHAR) MAC(STRING) MAC(INTEGER) MAC(REAL) MAC(BOOLEAN) MAC(BITSET)  \
    MAC(REC) MAC(ARRAY) MAC(TUPLE) MAC(HASH) MAC(SET) MAC(LIST)  \
    MAC(STRING_VAL) MAC(INT_VAL) MAC(REAL_VAL)  \
    MAC(TRUE) MAC(FALSE)  \
    MAC(MOD) MAC(DOM) MAC(FROM) MAC(IMPORT) MAC(EXTERN)  \
    MAC(COLON) MAC(SEMICOLON)  \
    MAC(COMMA)  \
    MAC(LBRACE) MAC(RBRACE) MAC(LBRACKET) MAC(RBRACKET) MAC(LPAREN) MAC(RPAREN)  \
    MAC(EQ) MAC(COLON_EQ) MAC(COLON_COLON)  \
    MAC(INC) MAC(DEC) MAC(PLUS_EQ) MAC(MINUS_EQ) MAC(MULT_EQ) MAC(DIV_EQ)  \
    MAC(PLUS) MAC(MINUS) MAC(MULT) MAC(DIV) MAC(MODULO)  \
    MAC(EQ_EQ) MAC(LT) MAC(GT) MAC(NOT_EQ) MAC(LT_EQ) MAC(GT_EQ)  \
    MAC(NOT) MAC(AND) MAC(OR) MAC(XOR)  \
    MAC(BIT_NOT) MAC(BIT_AND) MAC(BIT_OR) MAC(BIT_XOR)  \
    MAC(BEGIN) MAC(END)  \
    MAC(DOT) MAC(TICK)  \
    MAC(DOT_DOT) MAC(DOT_DOT_DOT) MAC(ST) MAC(IN)  \
    MAC(IDENTIFIER)  \
    MAC(LEXEOF)

enum token {
    FOREACH_TOKEN(GEN_ENUM)
};


struct token_st {
  enum token tok;
  char* val;
  int line_no;
};

#ifndef _LEXER_C_
extern const char *token_names[];
#else
const char *token_names[] = {
    FOREACH_TOKEN(GEN_STR)
};
#endif

typedef struct token_st* token_t;

/* Lex A File
 * file_name - Name of file to generate tokens from
 *
 * Generates a linked list of tokens from the given input file f.
 */
ll_t lex(char* file_name);

#endif

