#include <stdio.h>
#include "util.h"
#include "ast.h"

// arrays to index into for printing out enums
char type_kinds[][12] = {"TY_STRING", "TY_INTEGER", "TY_REAL", "TY_BOOLEAN",
           "TY_ARRAY", "TY_REC", "TY_HASH", "TY_LIST", "TY_NAME"};

char morph_paths[][12] = {"DIRECT_PATH", "BEST_PATH"};

// function to indent levels of the tree when printing
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
      fprintf(out, "Type: %s, %s\n", types->name, type_kinds [types->type->kind]);
      types = types->next;
    }
    mods = mods->next;
  }
}
