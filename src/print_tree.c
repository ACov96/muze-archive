#include <stdio.h>
#include <string.h>
#include "util.h"
#include "ast.h"

// arrays to index into for printing out enums
char type_kinds[][12] = {"STRING_TY", "INTEGER_TY", "REAL_TY", "BOOLEAN_TY",
                         "ARRAY_TY", "REC_TY", "HASH_TY", "LIST_TY", "NAME_TY", "MORPH_TY"};

char morph_paths[][12] = {"DIRECT_PATH", "BEST_PATH"};

// THESE CHARACTERS ARE 3 BYTES, BEWARE!
#define LRPIPE  "\u2500"
#define UDPIPE  "\u2502"
#define DRPIPE  "\u250c"
#define URPIPE  "\u2514"
#define ULPIPE  "\u2518"
#define UDRPIPE "\u251c"

#define OUT_FILE out
#define BUF_START indent_start
#define BUF_END indent_end
#define PARAMS FILE *OUT_FILE, char *BUF_START, char *BUF_END

#define INDT_APP(str, num) \
  strncpy(BUF_END, str, num + 1); \
  BUF_END += num

#define INDT_DROP(num) \
  BUF_END -= num; \
  *BUF_END = '\0'

#define PRINT_NODE(name, fn, node) \
  fprintf(OUT_FILE, "%s " UDRPIPE "%s\n", BUF_START, name); \
  INDT_APP(" " UDPIPE, 4); \
  if (node) { \
    fn(node, OUT_FILE, BUF_START, BUF_END); \
  } \
  else { \
    PRINT_NULL; \
  } \
  INDT_DROP(4)

#define PRINT_LAST(name, fn, node) \
  fprintf(OUT_FILE, "%s " URPIPE "%s\n", BUF_START, name); \
  INDT_APP("  ", 2); \
  if (node) { \
    fn(node, OUT_FILE, BUF_START, BUF_END); \
  } \
  else { \
    PRINT_NULL; \
  } \
  INDT_DROP(2)

#define PRINT_NEXT(name, fn, node) \
  fprintf(OUT_FILE, "%s " URPIPE "%s\n", BUF_START, name); \
  if (node) { \
    fprintf(OUT_FILE, "%s " DRPIPE LRPIPE ULPIPE "\n", BUF_START); \
    fn(node, OUT_FILE, BUF_START, BUF_END); \
  } \
  else { \
    fprintf(OUT_FILE, "%s   " URPIPE "]\n", BUF_START); \
  }

#define PRINT_LIT(fmt, ...) \
  fprintf(OUT_FILE, "%s " URPIPE fmt "\n", BUF_START, ##__VA_ARGS__)

#define PRINT_NULL \
  fprintf(OUT_FILE, "%s " URPIPE "]\n", BUF_START)

static void print_name(char *name, PARAMS);

static void print_root(root_t root, PARAMS);
static void print_mod(mod_t mod, PARAMS);
static void print_decl(decl_t decl, PARAMS);
static void print_const_decl(const_decl_t con, PARAMS);
static void print_type_decl(type_decl_t ty, PARAMS);
static void print_var_decl(var_decl_t var, PARAMS);
static void print_fun_decl(fun_decl_t fun, PARAMS);

static void print_name(char *name, PARAMS) {
  PRINT_LIT("'%s'", name);
}

static void print_const_decl(const_decl_t con, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_type_decl(type_decl_t ty, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_var_decl(var_decl_t var, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_fun_decl(fun_decl_t fun, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_decl(decl_t decl, PARAMS) {
  PRINT_NODE("Constants", print_const_decl, decl->constants);
  PRINT_NODE("Types", print_type_decl, decl->types);
  PRINT_NODE("Variables", print_var_decl, decl->vars);
  PRINT_NODE("Functions", print_fun_decl, decl->funs);
  PRINT_LAST("Modules", print_mod, decl->mods);
}

static void print_mod(mod_t mod, PARAMS) {
  PRINT_NODE("Name", print_name, mod->name);
  PRINT_NODE("Declarations", print_decl, mod->decl);
  PRINT_NEXT("Next", print_mod, mod->next);
}

static void print_root(root_t root, PARAMS) {
  fprintf(OUT_FILE, "Root\n");
  PRINT_LAST("Modules", print_mod, root->mods);
}

void print_tree(FILE *out, root_t root) {
  char buf_start[BUFSIZ], *buf_end;
  buf_end = buf_start;
  print_root(root, out, buf_start, buf_end);
}
