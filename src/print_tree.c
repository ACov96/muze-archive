#include <stdio.h>
#include "util.h"
#include "ast.h"

static void indent(FILE *out, int d) {
  for (int i = 0; i <= d; i++)
    fprintf(out, "  ");
}

void print_tree(FILE *out, root_t root, int d) {
  printf("\n-----Abstract Syntax Tree-----\n");
  // print mod blocks
  mod_t mods = root->mods;
  while(mods) {
    fprintf(out, "mod: %s\n", mods->name);
    decl_t decl = mods->decl;
    // print const declarations
    const_t cons = decl->constants;
    while(cons) {
      indent(out, d);
      fprintf(out, "CONST: %s\n", cons->name);
      cons = cons->next;
    }
    // print type declarations
    type_decl_t types = decl->types;
    while(types){
      indent(out, d);
      fprintf(out, "Type: %s\n", types->name);
      types = types->next;
    }
    mods = mods->next;
  }
}
