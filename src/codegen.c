#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"

unsigned int curr_label = 0;

char* gen_label() {
  // TODO: Make this malloc dynamic
  char *label = malloc(16);
  sprintf(label, "M%d", curr_label);
  printf("GENERATED LABEL: %s\n", label);
  curr_label++;
  return label;
}

char* codegen(root_t root) {
  gen_label();
  return "NOTHING";
}
