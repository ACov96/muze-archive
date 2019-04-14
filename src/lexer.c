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
  char** lines = split_string(s, "\n", &numLines);
  for (int i = 0; i < numLines; i++) {
    int numWords = 0;
    char** words = split_string(lines[i], " ", &numWords);
    for (int j = 0; j < numWords; j++)
      ll_append(token_list, token_from_word(words[j]));
  }
  return token_list;
}

token_t token_from_word(char* s) {
  char* buf = malloc(strlen(s));
  strcpy(buf, s);
  if (strcmp(buf, "mod") == 0)
    return new_token(MOD, "mod");
  else if (strcmp(buf, "dom") == 0)
    return new_token(DOM, "dom");
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
