#include <stdio.h>
#include "util.h"
#include "ast.h"

static void indent(FILE *out, int d) {
  for (int i = 0; i <= d; i++)
    fprintf(out, "  ");
}

void print_tree(FILE *out, root_t root, int d) {
  printf("-----Abstract Syntax Tree-----\n");
  //root_t r = root;
  mod_t mods = root->mods;
  while(mods) {
    fprintf(out, "mod: %s\n", mods->name);
    decl_t decl = mods->decl;
    const_t cons = decl->constants;
    //fprintf(out, "CONST: %s\n", cons->name);
    while(cons) {
      indent(out, d);
      fprintf(out, "CONST: %s", cons->name);
      if (cons->next == NULL)
        break;
      else
        cons = cons->next;
    }
    mods = mods->next;
  }
}
