#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

// Don't read just go on and trust it works
#define GEN_ENUM(ident) ident,
#define GEN_STR(ident) #ident,

#define FOREACH_TOKEN(A, Q) \
    Q(IF) Q(FI) Q(ELIF) Q(ELSE) Q(THEN) \
    Q(CASE) Q(ESAC) Q(OF) \
    Q(FOR) Q(ROF) Q(LOOP) Q(POOL) Q(BREAK) Q(CONTINUE) Q(CYCLE) Q(FA) Q(AF) \
    Q(FUN) Q(NUF)  \
    Q(TYPE) Q(EPYT) Q(MU) Q(UM) \
    Q(CONST) Q(VAR) \
    Q(CHAR) Q(STRING) Q(INTEGER) Q(REAL) Q(BOOLEAN) Q(BITSET) \
    Q(REC) Q(ARRAY) Q(TUPLE) Q(HASH) Q(SET) Q(LIST) \
    A(STRING_VAL, "string literal") A(INT_VAL, "integer literal") \
    A(REAL_VAL, "real literal") \
    Q(TRUE) Q(FALSE) \
    Q(MOD) Q(DOM) Q(FROM) Q(IMPORT) Q(EXTERN)  \
    A(COLON, ":") A(SEMICOLON, ";") \
    Q(COMMA, ",") \
    A(LBRACE, "{") A(RBRACE, "}") \
    A(LBRACKET, "[") A(RBRACKET, "]") \
    A(LPAREN, "(") A(RPAREN, ")") \
    A(EQ) A(COLON_EQ) A(COLON_COLON)  \
    Q(INC) Q(DEC) Q(PLUS_EQ) Q(MINUS_EQ) Q(MULT_EQ) Q(DIV_EQ) \
    Q(PLUS) Q(MINUS) Q(MULT) Q(DIV) Q(MODULO) \
    Q(EQ_EQ) Q(LT) Q(GT) Q(NOT_EQ) Q(LT_EQ) Q(GT_EQ) \
    Q(NOT) Q(AND) Q(OR) Q(XOR) \
    Q(BIT_NOT) Q(BIT_AND) Q(BIT_OR) Q(BIT_XOR) \
    Q(BEGIN) Q(END) \
    Q(DOT) Q(TICK) \
    Q(DOT_DOT) Q(DOT_DOT_DOT) Q(ARROW) Q(ST) Q(IN) \
    Q(IDENTIFIER) \
    Q(LEXEOF)

enum token {
    FOREACH_TOKEN(GEN_ENUM)
};


struct token_st {
  enum token tok;
  char* val;
  int line_no;
};

#ifdef _LEXER_C_
const char *token_names[] = {
    FOREACH_TOKEN(GEN_STR)
};
#else
extern const char *token_names[];
#endif

typedef struct token_st* token_t;

/* Lex A File
 * file_name - Name of file to generate tokens from
 *
 * Generates a linked list of tokens from the given input file f.
 */
ll_t lex(char* file_name);

#endif

