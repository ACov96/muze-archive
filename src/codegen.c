// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"

unsigned int curr_label = 0;

/* PROTOTYPES */
char* gen_mod(mod_t mod);
char* gen_fun(fun_decl_t fun);

char* gen_label() {
  // TODO: Make this malloc dynamic
  char *label = malloc(16);
  sprintf(label, "M%d", curr_label);
  printf("GENERATED LABEL: %s\n", label);
  curr_label++;
  return label;
}

char* gen_mod(mod_t mod) {
  char *buf = "";
  // Allocate module constants

  // Allocate module variables

  // Define type morph functions

  // Define functions
  for (fun_decl_t f = mod->decl->funs; f; f = f->next) {
    buf = concat(buf, gen_fun(f));
  }
  return buf;
}

char* gen_fun(fun_decl_t fun) {
  char *label = malloc(strlen(fun->name) + 2);
  sprintf(label, "%s:\n", fun->name);
  printf("LABEL: %s", label);
  return "";
}

char* codegen(root_t root) {
  char *assembly = "\t.global main\n";
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    assembly = concat(assembly, gen_mod(mod));
  }
  return assembly;
}
