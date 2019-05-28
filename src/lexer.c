#define _LEXER_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "util.h"
#include "limits.h"

// Peek n characters beyond the current character
#define peek(n) (i + n < strlen(s) ? s[i + n] : '\0')
#define new_token(t, val) _new_token(t, val, line_no, col_no)
#define inc(n) \
  for (int _j=1; _j<=n; _j++) { \
    if (peek(_j) == '\n') { \
      line_no++; \
      col_no = 0; \
    }  \
    else { \
      col_no++; \
    } \
  } \
  i += n;

/* PROTOTYPES */
char* file_to_string(FILE *f);
token_t _new_token(enum token t, char* val, int line_no, int col_no);
ll_t generate_token_list(char* s);
char** split_string(char* s, char* del, int* size);
token_t token_from_word(char* s);

int line_no = 1;
int col_no = 1;
int tok_col = 1;

ll_t lex(char* file_name) {
  FILE *f = fopen(file_name, "r");
  if (f == NULL)
    error_and_exit("Cannot open file", line_no);

  char* content = file_to_string(f);
  return generate_token_list(content);
}

ll_t generate_token_list(char* s) {
  ll_t token_list = ll_new();
  // Loop through file
  for (int i = 0; i < strlen(s); i++, col_no++, tok_col = col_no) {
    // set current character to c
    char c = s[i];
    // Whitespace, tab, new line
    if (c == ' ' || c == '\t') {
      continue;
    }
    else if (c == '\n') {
      line_no++;
      col_no = 0;
      tok_col = col_no;
      continue;
    }
    // check for (definite) single character symbols
    // {, }, ), ), ;, [, ]
    if (c == ';')
      ll_append(token_list, new_token(SEMICOLON, ";"));
    else if (c == ',')
      ll_append(token_list, new_token(COMMA, ","));
    else if (c == '{')
      ll_append(token_list, new_token(LBRACE, "{"));
    else if (c == '}')
      ll_append(token_list, new_token(RBRACE, "}"));
    else if (c == '(')
      ll_append(token_list, new_token(LPAREN, "("));
    else if (c == ')')
      ll_append(token_list, new_token(RPAREN, ")"));
    else if (c == '[')
      ll_append(token_list, new_token(LBRACKET, "["));
    else if (c == ']')
      ll_append(token_list, new_token(RBRACKET, "]"));

    else if (c == ':') {
      if (peek(1) == ':') {
        ll_append(token_list, new_token(COLON_COLON, "::"));
        inc(1);
      }
      else if (peek(1) == '=') {
        ll_append(token_list, new_token(COLON_EQ, ":="));
      }
      else {
        ll_append(token_list, new_token(COLON, ":"));
      }
    }

    // arithmetic operators
    else if (c == '+') {
      if (peek(1) == '+') {
        ll_append(token_list, new_token(INC, "++"));
        inc(1);
      }
      else if (peek(1) == '=') {
        ll_append(token_list, new_token(PLUS_EQ, "+="));
        inc(1);
      } else
        ll_append(token_list, new_token(PLUS, "+"));
    }
    else if (c == '-') {
      if (peek(1) == '-') {
        ll_append(token_list, new_token(DEC, "--"));
        inc(1);
      }
      else if (peek(1) == '=') {
        ll_append(token_list, new_token(MINUS_EQ, "-="));
        inc(1);
      }
      else if (peek(1) == '>') {
        ll_append(token_list, new_token(ARROW, "->"));
        inc(1);
      }else
        ll_append(token_list, new_token(MINUS, "-"));
    }
    else if (c == '*') {
      if (peek(1) == '=') {
        ll_append(token_list, new_token(MULT_EQ, "*="));
        inc(1);
      } else
        ll_append(token_list, new_token(MULT, "*"));
    }
    else if (c == '/') {
      if (peek(1) == '=') {
        ll_append(token_list, new_token(DIV_EQ, "/="));
        inc(1);
      } else if (s[i+1] == '*') {
        /* Multi-line
         * comments */
        // Allow for nested comments
        int levels_deep = 0;
        while (1) {
          inc(1);
          if (peek(2))
            error_and_exit("Reached end of file in unclosed comment", line_no);
          if (peek(1) == '/' && peek(2) == '*')
            levels_deep++;
          else if (peek(1) == '*' && peek(2) == '/') {
            if (levels_deep == 0) {
              inc(2);
              break;
            } else
              levels_deep--;
          }
        }
      }
      else if (peek(1) == '/') // Single line comments
         do {
             inc(1);
         } while (s[i] != '\n' && s[i] != '\0');
      else
        ll_append(token_list, new_token(DIV, "/"));
    }

    // Comparison operators
    else if (c == '<') {
      if (peek(1) == '=') {
        inc(1);
        ll_append(token_list, new_token(LT_EQ, "<="));
      } else 
        ll_append(token_list, new_token(LT, "<"));
    }

    else if (c == '>') {
      if (peek(1) == '=') {
        inc(1);
        ll_append(token_list, new_token(GT_EQ, ">="));
      } else 
        ll_append(token_list, new_token(GT, ">"));
    }

    else if (c == '!') {
      if (peek(1) == '=') {
        inc(1);
        ll_append(token_list, new_token(NOT_EQ, "!="));
      } else 
        ll_append(token_list, new_token(NOT, "!"));
    }
    
    else if (c == '=') {
      if (peek(1) == '=') {
        inc(1);
        ll_append(token_list, new_token(EQ_EQ, "=="));
      } else 
        ll_append(token_list, new_token(EQ, "="));
    }

    // boolean
    else if (c == '&') {
      if (peek(1) == '&') {
        ll_append(token_list, new_token(AND, "&&"));
        inc(1);
      }
      else {
        ll_append(token_list, new_token(BIT_AND, "&"));
      }
    }

    else if (c == '|') {
      if (peek(1) == '|') {
        ll_append(token_list, new_token(OR, "||"));
        inc(1);
      }
      else {
        ll_append(token_list, new_token(BIT_OR, "|"));
      }
    }

    else if (c == '^') {
      if (peek(1) == '^') {
        ll_append(token_list, new_token(XOR, "^^"));
        inc(1);
      }
      else {
        ll_append(token_list, new_token(BIT_XOR, "^"));
      }
    }

    else if (c == '~') {
      ll_append(token_list, new_token(BIT_NOT, "~"));
    }

    // dots
    else if (c == '.') {
      if (peek(1) == '.') {
        if (peek(2) == '.') {
          ll_append(token_list, new_token(DOT_DOT_DOT, "..."));
          inc(2);
        }
        else {
          ll_append(token_list, new_token(DOT_DOT, ".."));
          inc(1);
        }
      } else
        ll_append(token_list, new_token(DOT, "."));
    }
    
    else if (c == '\'') {
      ll_append(token_list, new_token(TICK, "'"));
    }

    // check for alpha character.
    // If alpha loop until non alphanumeric
    // Then pass to token_from_word
    // Handles reserved words and identifiers
    // don't forget to check for periods and ticks
    else if (isalpha(c)) {
      char *id = malloc(MAX_IDENTIFIER_SIZE + 1);
      memset(id, 0, (MAX_IDENTIFIER_SIZE + 1) * sizeof(char));
      int j = 0;
      id[j] = c;
      j++;

      while (isalpha(peek(1)) || isdigit(peek(1)) || peek(1) == '_') {
        if (j > MAX_IDENTIFIER_SIZE - 1)
          error_and_exit("Variable name cannot exceed 255 characters.", line_no);
        id[j] = peek(1);
        j++;
        inc(1);
      }

      // Just in case anyone actually reads the commit diff:
      // I am very unhappy with whoever decided to previously write 'i--' here.
      // I just spent half an hour trying to figure out why none of the line
      // numbers were being tracked correctly, and it was because we were, just
      // in this one spot, moving the current char BACKWARDS.
      //
      // You owe me a beer

      ll_append(token_list, token_from_word(id));
    }

    // Check for digits
    // don't forget about periods for floats
    else if (isdigit(c)) {
      char *id = malloc(MAX_NUMBER_SIZE + 1);
      memset(id, 0, (MAX_NUMBER_SIZE + 1) * sizeof(char));

      int j = 0;
      int is_float = 0;
      while (isdigit(s[i]) || (s[i] == '.' && peek(1) == '.')) {
        if (s[i] == '.' && is_float == 1)
          error_and_exit("Invalid real.", line_no);
        else if (s[i] == '.')
          is_float = 1;
        if (j > MAX_NUMBER_SIZE - 1)
          error_and_exit("exceeded digit buffer.", line_no);
        id[j] = s[i];
        j++;
        inc(1);
      }
      i--;
      if (is_float == 1)
        ll_append(token_list, new_token(REAL_VAL, id));
      else
        ll_append(token_list, new_token(INT_VAL, id));
    }
    // Check for strings
    else if (c == '"') {
      char *buf = NULL, *tmp = NULL;
      unsigned int bufLen = 0;
      inc(1);
      while(s[i] != '"') {
        tmp = realloc(buf, bufLen+1);
        buf = tmp;
        buf[bufLen] = s[i];
        bufLen++;
        inc(1);
      }
      tmp = realloc(buf, bufLen+1);
      buf = tmp;
      ll_append(token_list, new_token(STRING_VAL, buf));
    }

    // Some weird character, error out
    else {
      error_and_exit("Unrecognized character", line_no);
    }
  }

  // Get an ending token
  ll_append(token_list, new_token(LEXEOF, ""));

  return token_list;
}

token_t token_from_word(char* s) {
  char* buf = malloc(strlen(s));
  strcpy(buf, s);

  // Module tokens
  if (strcmp(buf, "mod") == 0)
    return new_token(MOD, "mod");
  else if (strcmp(buf, "dom") == 0)
    return new_token(DOM, "dom");
  else if (strcmp(buf, "from") == 0)
    return new_token(FROM, "from");
  else if (strcmp(buf, "import") == 0)
    return new_token(IMPORT, "import");
  else if (strcmp(buf, "extern") == 0)
    return new_token(EXTERN, "extern");

  // Conditional tokens
  else if (strcmp(buf, "if") == 0)
    return new_token(IF, "if");
  else if (strcmp(buf, "fi") == 0)
    return new_token(FI, "fi");
  else if (strcmp(buf, "elif") == 0)
    return new_token(ELIF, "elif");
  else if (strcmp(buf, "else") == 0)
    return new_token(ELSE, "else");
  else if (strcmp(buf, "then") == 0)
    return new_token(THEN, "then");
  else if (strcmp(buf, "st") == 0)
    return new_token(ST, "st");
  else if (strcmp(buf, "in") == 0)
    return new_token(IN, "in");

  // Loop tokens
  else if (strcmp(buf, "for") == 0)
    return new_token(FOR, "for");
  else if (strcmp(buf, "rof") == 0)
    return new_token(ROF, "rof");
  else if (strcmp(buf, "fa") == 0)
    return new_token(FA, "fa");
  else if (strcmp(buf, "af") == 0)
    return new_token(AF, "af");
  else if (strcmp(buf, "loop") == 0)
    return new_token(LOOP, "loop");
  else if (strcmp(buf, "pool") == 0)
    return new_token(POOL, "pool");
  else if (strcmp(buf, "break") == 0)
    return new_token(BREAK, "break");
  else if (strcmp(buf, "continue") == 0)
    return new_token(CONTINUE, "continue");
  else if (strcmp(buf, "cycle") == 0)
    return new_token(CYCLE, "cycle");

  // Case tokens
  else if (strcmp(buf, "case") == 0)
    return new_token(CASE, "case");
  else if (strcmp(buf, "esac") == 0)
    return new_token(ESAC, "esac");
  else if (strcmp(buf, "of") == 0)
    return new_token(OF, "of");

  // Function tokens
  else if (strcmp(buf, "fun") == 0)
    return new_token(FUN, "fun");
  else if (strcmp(buf, "nuf") == 0)
    return new_token(NUF, "nuf");

  // Type tokens
  else if (strcmp(buf, "type") == 0)
    return new_token(TYPE, "type");
  else if (strcmp(buf, "mu") == 0)
    return new_token(MU, "mu");
  else if (strcmp(buf, "um") == 0)
    return new_token(UM, "um");

  //CONST and VAR tokens
  else if (strcmp(buf, "const") == 0)
    return new_token(CONST, "const");
  else if (strcmp(buf, "var") == 0)
    return new_token(VAR, "var");

  // Basic types
  else if (strcmp(buf, "string") == 0)
    return new_token(STRING, "string");
  else if (strcmp(buf, "integer") == 0)
    return new_token(INTEGER, "integer");
  else if (strcmp(buf, "real") == 0)
    return new_token(REAL, "real");
  else if (strcmp(buf, "boolean") == 0)
    return new_token(BOOLEAN, "boolean");
  else if (strcmp(buf, "bitset") == 0)
    return new_token(BITSET, "bitset");
  else if (strcmp(buf, "true") == 0)
    return new_token(TRUE, "true");
  else if (strcmp(buf, "false") == 0)
    return new_token(FALSE, "false");

  // Composite types
  else if (strcmp(buf, "rec") == 0)
    return new_token(REC, "rec");
  else if (strcmp(buf, "array") == 0)
    return new_token(ARRAY, "array");
  else if (strcmp(buf, "tuple") == 0)
    return new_token(TUPLE, "tuple");
  else if (strcmp(buf, "hash") == 0)
    return new_token(HASH, "hash");
  else if (strcmp(buf, "set") == 0)
    return new_token(SET, "set");
  else if (strcmp(buf, "list") == 0)
    return new_token(LIST, "list");

  // Block tokens
  else if (strcmp(buf, "begin") == 0)
    return new_token(BEGIN, "begin");
  else if (strcmp(buf, "end") == 0)
    return new_token(END, "end");

  // None of the above, so probably an identifier
  else
    return new_token(IDENTIFIER, buf);
}

char** split_string(char* s, char* del, int *size) {
  int num = 0;
  char* copy = malloc(strlen(s));
  strcpy(copy, s);
  char* string = strtok(copy, del);

  // Count the total number of strings
  while (string) {
    num++;
    string = strtok(NULL, del);
  }

  // Allocate enough space and move the strings into their own array
  char** strings = malloc(sizeof(char*) * num);
  strcpy(copy, s);
  string = strtok(copy, del);
  for (int i = 0; i < num; i++) {
    strings[i] = string;
    string = strtok(NULL, "\n");
  }
  *size = num;
  return strings;
}

token_t _new_token(enum token t, char* val, int line_no, int col_no) {
  token_t tok = malloc(sizeof(struct token_st));
  tok->tok = t;
  tok->val = malloc(strlen(val));
  strcpy(tok->val, val);
  tok->line_no = line_no;
  tok->col_no = tok_col;
  tok_col = col_no;
  return tok;
}

char* file_to_string(FILE *f) {
  fseek(f, 0, SEEK_END);
  int file_length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buffer = malloc(file_length);
  fread(buffer, 1, file_length, f);
  return buffer;
}

