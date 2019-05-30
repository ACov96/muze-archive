#ifndef _LEXER_H
#define _LEXER_H

#include "util.h"

// Don't read just go on and trust it works
#define GEN_ENUM(ident, x) ident
#define GEN_ASSOC(ident, str) {#ident, str}

#define FOREACH_TOKEN(A) \
    A(IF, "'if'"), A(FI, "'fi'"), A(ELIF, "'elif'"), A(ELSE, "'else'"), A(THEN, "'then'"), \
    A(CASE, "'case'"), A(ESAC, "'esac'"), A(OF, "'of'"), \
    A(FOR, "'for'"), A(ROF, "'rof'"), \
    A(FA, "'fa'"), A(AF, "'af'"), \
    A(LOOP, "'loop'"), A(POOL, "'pool'"), \
    A(BREAK, "'break'"), A(CONTINUE, "'continue'"), A(CYCLE, "'cycle'"), \
    A(FUN, "'fun'"), A(NUF, "'nuf'"),  \
    A(TYPE, "'type'"), A(EPYT, "'epyt'"), A(MU, "'mu'"), A(UM, "'um'"), \
    A(CONST, "'const'"), A(VAR, "'var'"), \
    A(STRING, "'string'"), A(INTEGER, "'integer'"), A(REAL, "'real'"), \
    A(BOOLEAN, "'boolean'"), A(BITSET, "'bitset'"), \
    A(REC, "'rec'"), A(CER, "'cer'"), \
    A(ARRAY, "'array'"), A(TUPLE, "'tuple'"), A(HASH, "'hash'"), \
    A(SET, "'set'"), A(LIST, "'list'"), \
    A(STRING_VAL, "string literal"), A(INT_VAL, "integer literal"), \
    A(REAL_VAL, "real literal"), \
    A(TRUE, "'true'"), A(FALSE, "'false'"), \
    A(MOD, "'mod'"), A(DOM, "'dom'"), A(FROM, "'from'"), \
    A(IMPORT, "'import'"), A(EXTERN, "'extern'"),  \
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
    A(BEGIN, "'begin'"), A(END, "'end'"), \
    A(DOT, "'.'"), A(TICK, "'''"), \
    A(DOT_DOT, "'..'"), A(DOT_DOT_DOT, "'...'"), \
    A(ARROW, "'->'"), \
    A(ST, "'st'"), A(IN, "'in'"), \
    A(IDENTIFIER, "identifier"), \
    A(LEXEOF, "EOF")

enum token {
    FOREACH_TOKEN(GEN_ENUM)
};

struct tokname_st {
  char *raw;
  char *pretty;
};

#ifdef _LEXER_C_
const struct tokname_st token_names[] = {
    FOREACH_TOKEN(GEN_ASSOC)
};
#else
extern const struct tokname_st token_names[];
#endif

struct token_st {
  enum token tok;
  char* val;
  int line_no;
  int col_no;
};

typedef struct token_st* token_t;

/* Lex A File
 * file_name - Name of file to generate tokens from
 *
 * Generates a linked list of tokens from the given input file f.
 */
ll_t lex(char* file_name);

void print_tokens(ll_t toks);

#endif

