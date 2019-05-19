#include <stdio.h>
#include "util.h"
#include "ast.h"

// arrays to index into for printing out enums
char type_kinds[][12] = {"STRING_TY", "INTEGER_TY", "REAL_TY", "BOOLEAN_TY",
                         "ARRAY_TY", "REC_TY", "HASH_TY", "LIST_TY", "NAME_TY", "MORPH_TY"};

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
      fprintf(out, "const: %s\n", cons->name);
      cons = cons->next;
    }
    // print type declarations
    type_decl_t types = decl->types;
    while(types) {
      indent(out, d);
      fprintf(out, "type: %s, %s\n", types->name, type_kinds [types->type->kind]);
      types = types->next;
    }
    var_t vars = decl->vars;
    while(vars) {
      indent(out, d);
      fprintf(out, "var: %s, type: %s\n", vars->name, type_kinds[vars->type->kind]);
      vars= vars->next;
    }
    mods = mods->next;
  }
}
