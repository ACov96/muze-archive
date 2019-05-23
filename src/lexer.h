#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

// Don't read just go on and trust it works
#define GEN_ENUM2(ident, x) ident
#define GEN_ENUM(ident) ident
#define GEN_QUOT(ident) {#ident, "'"#ident"'"}
#define GEN_ASSOC(ident, str) {#ident, str}

#define FOREACH_TOKEN(Q, A) \
    Q(IF), Q(FI), Q(ELIF), Q(ELSE), Q(THEN), \
    Q(CASE), Q(ESAC), Q(OF), \
    Q(FOR), Q(ROF), Q(LOOP), Q(POOL), Q(BREAK), Q(CONTINUE), Q(CYCLE), Q(FA), Q(AF), \
    Q(FUN), Q(NUF),  \
    Q(TYPE), Q(EPYT), Q(MU), Q(UM), \
    Q(CONST), Q(VAR), \
     Q(STRING), Q(INTEGER), Q(REAL), Q(BOOLEAN), Q(BITSET), \
    Q(REC), Q(CER), \
    Q(ARRAY), Q(TUPLE), Q(HASH), Q(SET), Q(LIST), \
    A(STRING_VAL, "string literal"), A(INT_VAL, "integer literal"), \
    A(REAL_VAL, "real literal"), \
    Q(TRUE), Q(FALSE), \
    Q(MOD), Q(DOM), Q(FROM), Q(IMPORT), Q(EXTERN),  \
    A(COLON, "':'"), A(SEMICOLON, "';'"), \
    A(COMMA, "','"), \
    A(LBRACE, "'{'"), A(RBRACE, "'}'"), \
    A(LBRACKET, "'['"), A(RBRACKET, "']'"), \
    A(LPAREN, "'('"), A(RPAREN, "'),'"), \
    A(EQ, "'='"), A(COLON_EQ, "':='"), A(COLON_COLON, "'::'"),  \
    A(PLUS_EQ, "'+='"), A(MINUS_EQ, "'-='"), A(MULT_EQ, "'*='"), \
    A(DIV_EQ, "'/='"), A(MOD_EQ, "'%='"), \
    A(OR_EQ, "'|='"), A(AND_EQ, "'&='"), A(XOR_EQ, "'^='"), \
    A(INC, "'++'"), A(DEC, "'--'"), \
    A(PLUS, "'+'"), A(MINUS, "'-'"), A(MULT, "'*'"), A(DIV, "'/'"), A(MODULO, "'%'"), \
    A(EQ_EQ, "'=='"), A(LT, "'<'"), A(GT, "'>'"), \
    A(NOT_EQ, "'!='"), A(LT_EQ, "'<='"), A(GT_EQ, "'>='"), \
    A(NOT, "'!'"), A(AND, "'&&'"), A(OR, "'||'"), A(XOR, "'^^'"), \
    A(BIT_NOT, "'~'"), A(BIT_AND, "'&'"), A(BIT_OR, "'|'"), A(BIT_XOR, "'^'"), \
    Q(BEGIN), Q(END), \
    A(DOT, "'.'"), A(TICK, "'''"), \
    A(DOT_DOT, "'..'"), A(DOT_DOT_DOT, "'...'"), \
    A(ARROW, "'->'"), \
    Q(ST), Q(IN), \
    A(IDENTIFIER, "identifier"), \
    Q(LEXEOF)

enum token {
    FOREACH_TOKEN(GEN_ENUM, GEN_ENUM2)
};

struct tokname_st {
  char *raw;
  char *pretty;
};

#ifdef _LEXER_C_
const struct tokname_st token_names[] = {
    FOREACH_TOKEN(GEN_QUOT, GEN_ASSOC)
};
#else
extern const struct tokname_st token_names[];
#endif

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

