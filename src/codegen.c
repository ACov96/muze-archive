// TODO: This module is memory-leak central and just leaks strings all over. Sorry.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "util.h"

#define WORD 8
#define CREATE_BUFFER char *buf = ""
#define RETURN_BUFFER return buf
#define PRINT_BUFFER printf("%s\n", buf)

#define ADD_BLOCK(B) buf = concat(buf, B)

#define ADD_LABEL(L) buf = concat(buf, L); \
  buf = concat(buf, ":\n")

#define ADD_INSTR(I, O) buf = concat(buf, "\t");  \
  buf = concat(buf, I); \
  buf = concat(buf, "\t"); \
  buf = concat(buf, O); \
  buf = concat(buf, "\n")

typedef struct label_st *label_t;

struct label_st {
  char         *label;
  unsigned int  count;
};

/* GLOBALS */
unsigned int curr_label = 0;
ll_t labels;

/* PROTOTYPES */
// Helper functions
unsigned int count_consts_and_vars(decl_t decl);
char* gen_label(char *label);
int get_id_offset(char *id, decl_t decl);

// Generator functions
char* gen_mod(mod_t mod);
char* gen_fun(fun_decl_t fun);

/* HELPERS */
char* gen_label(char *label) {
  char *new_label = NULL;
  if (!labels) {
    labels = ll_new(); 
    label_t l = malloc(sizeof(struct label_st));
    l->label = label;
    l->count = 0;
    new_label = concat(label, itoa(l->count));
    ll_append(labels, l);
  } else {
    for (ll_t l = labels; l; l = l->next) {
      label_t curr = l->val;
      if (strcmp(curr->label, label) == 0) {
        curr->count++;
        new_label = concat(label, itoa(curr->count));
      }
    }  
    if (!new_label) {
      // We didn't find the label we were looking for in the linked list, create it
      label_t l = malloc(sizeof(struct label_st));
      l->label = label;
      l->count = 0;
      new_label = concat(label, itoa(l->count));
      ll_append(labels, l);
    }
  }
  return new_label;
}

unsigned int count_consts_and_vars(decl_t decl) {
  unsigned int total = 0;
  for (const_decl_t c = decl->constants; c; c = c->next)
    total++;
  for (var_decl_t var = decl->vars; var; var = var->next)
    for (id_list_t id = id; id; id = id->next)
      total++;
  return total * WORD;
}

int get_id_offset(char *id, decl_t decl) {
  int offset = 0;
  for (const_decl_t c = decl->constants; c; c = c->next) {
    if (strcmp(id, c->name) == 0) return offset;
    offset++;
  }
  for (var_decl_t v = decl->vars; v; v = v->next) {
    for (id_list_t i = v->names; i; i = i->next) {
      if (strcmp(i->name, id) == 0) return offset;
      offset++;
    }
  }
  // TODO: Change this to an actual error message and quit
  return -1;
}

/* GENERATORS */
char* gen_mod(mod_t mod) {
  CREATE_BUFFER;
  // Allocate module constants

  // Allocate module variables

  // Define type morph functions

  // Define functions
  printf("%s\n", gen_label("L"));
  printf("%s\n", gen_label("S"));
  printf("%s\n", gen_label("L"));
  printf("%s\n", gen_label("S"));
  /* for (fun_decl_t f = mod->decl->funs; f; f = f->next) { */
  /*   buf = concat(buf, gen_fun(f)); */
  /* } */
  RETURN_BUFFER;
}

char* gen_fun(fun_decl_t fun) {
  unsigned int preamble_space = count_consts_and_vars(fun->decl);
  char *sub_inst = malloc(32);
  sprintf(sub_inst, "$%d, %%rsp", preamble_space);
  CREATE_BUFFER;
  ADD_LABEL(fun->name);
  ADD_INSTR("movq", "%rsp, %rbp");
  ADD_INSTR("sub", sub_inst);
  RETURN_BUFFER;
}

char* codegen(root_t root) {
  char *assembly = "\t.global main\n";
  for (mod_t mod = root->mods; mod; mod = mod->next) {
    assembly = concat(assembly, gen_mod(mod));
  }
  return assembly;
}
