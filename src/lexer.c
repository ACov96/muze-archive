#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "util.h"

/* PROTOTYPES */
char* file_to_string(FILE *f);
token_t new_token(enum token t, char* val);
ll_t generate_token_list(char* s);
char** split_string(char* s, char* del, int* size);
token_t token_from_word(char* s);

ll_t lex(char* file_name) {
  FILE *f = fopen(file_name, "r");
  if (f == NULL)
    error_and_exit("Cannot open file");

  char* content = file_to_string(f);
  return generate_token_list(content);
}

ll_t generate_token_list(char* s) {
  int numLines = 0;
  ll_t token_list = ll_new();
  // Loop through file
  for (int i = 0; i < strlen(s); i++) {
    // set current character to c
    char c = s[i];
    // Whitespace, tab, new line
    if (c == ' ' || c == '\t' || c == '\n')
      continue;

    // check for (definite) single character symbols
    // {, }, ), ), ;, [, ]
    if (c == ';')
      ll_append(token_list, new_token(SEMICOLON, ";"));
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



    // operators (all cases)
    else if (c == '+') {
      if (i+1 < strlen(s) && s[i+1] == '+') {
        ll_append(token_list, new_token(INC, "++"));
        i++;
      }
      else if (i+1 < strlen(s) && s[i+1] == '=') {
        ll_append(token_list, new_token(PLUS_EQ, "+="));
        i++;
      } else
        ll_append(token_list, new_token(PLUS, "+"));
    }
    else if (c == '-') {
      if (i+1 < strlen(s) && s[i+1] == '-') {
        ll_append(token_list, new_token(DEC, "--"));
        i++;
      }
      else if (i+1 < strlen(s) && s[i+1] == '=') {
        ll_append(token_list, new_token(MINUS_EQ, "-="));
        i++;
      } else
        ll_append(token_list, new_token(MINUS, "-"));
    }
    else if (c == '*') {
      if (i+1 < strlen(s) && s[i+1] == '=') {
        ll_append(token_list, new_token(MULT_EQ, "*="));
        i++;
      } else
        ll_append(token_list, new_token(MULT, "*"));
    }
    else if (c == '/') {
      if (i+1 < strlen(s) && s[i+1] == '=') {
        ll_append(token_list, new_token(DIV_EQ, "/="));
        i++;
      } else if (s[i+1] == '*') {
        /* Multi-line
         * comments */
        // Allow for nested comments
        int levels_deep = 0;
        while (true) {
          i++;
          if (s[i] == '/' && s[i+1] == '*')
            levels_deep++;
          if (i+2 < strlen(s))
            error_and_exit("Reached end of file in unclosed comment");
          if (s[i+1] == '*' && s[i+2] == '/' && levels_deep == 0) {
            i += 2;
            break;
          }
        }
      }
      else if (s[i+1] == '/') // Single line comments
        i++;
      else
        ll_append(token_list, new_token(DIV, "/"));
    }

    // check for alpha character.
    // If alpha loop until non alphanumeric
    // Then pass to token_from_word
    // Handles reserved words and identifiers
    // don't forget to check for periods and ticks
    else if (isalpha(c)) {
      char id[256];
      int j = 0;
      id[j] = s[i];
      while (isalpha(s[i+1]) || isdigit(s[i+1]) || s[i+1] == '_') {
        if (j > 254)
          error_and_exit("Variable name cannot exceed 254 characters.");
        id[j] = s[i+1];
        j++;
        i++;
      }
      ll_append(token_list, token_from_word(id));
    }

    // Check for digits
    // don't forget about periods for floats
    else if (isdigit(c)) {
      char id[256];
      int j = 0;
      id[j] = s[i];
      int is_float = 0;
      while (isdigit(s[i+1]) || s[i+1] == '.') {
        if (s[i+1] == '.' && is_float == 1)
          error_and_exit("Invalid real.");
        else if (s[i+1] == '.')
          is_float = 1;
        if (j > 254)
          error_and_exit("exceeded digit buffer.");
        id[j] = s[i+1];
        j++;
        i++;
      }
      if (is_float == 1)
        ll_append(token_list, new_token(REAL, atof(id)))
      else
        ll_append(token_list, new_token(INT, atoi(id)))
    }
    // Check for strings
    else if (c == '"') {

    }

    // Some weird character, error out
    else {
      error_and_exit("Unrecognized character");
    }
  }
  return token_list;
}

token_t token_from_word(char* s) {
  char* buf = malloc(strlen(s));
  strcpy(buf, s);
  //module tokens
  if (strcmp(buf, "mod") == 0)
    return new_token(MOD, "mod");
  else if (strcmp(buf, "dom") == 0)
    return new_token(DOM, "dom");
  // if/elif/else/then tokens
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
   // loop tokens
  else if (strcmp(buf, "for") == 0)
    return new_token(FOR, "for");
  else if (strcmp(buf, "rof") == 0)
    return new_token(ROF, "rof");
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
  // case tokens
  else if (strcmp(buf, "case") == 0)
    return new_token(CASE, "case");
  else if (strcmp(buf, "esac") == 0)
    return new_token(ESAC, "esac");
  // function tokens
  else if (strcmp(buf, "fun") == 0)
    return new_token(FUN, "fun");
  else if (strcmp(buf, "nuf") == 0)
    return new_token(NUF, "nuf");
  // type tokens
  else if (strcmp(buf, "type") == 0)
    return new_token(TYPE, "type");
  else if (strcmp(buf, "mu") == 0)
    return new_token(MU, "mu");
  // operator tokens
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

token_t new_token(enum token t, char* val) {
  token_t tok = malloc(sizeof(struct token_st));
  tok->tok = t;
  tok->val = val;
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
