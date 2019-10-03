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

#define PRINT_KIND(name, fn, node) \
  fprintf(OUT_FILE, "%s " UDRPIPE "%s\n", BUF_START, name); \
  INDT_APP(" " UDPIPE, 4); \
  fn(node, OUT_FILE, BUF_START, BUF_END); \
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

static void print_decl(decl_t decl, PARAMS);
static void print_const_decl(const_decl_t con, PARAMS);
static void print_type_decl(type_decl_t ty, PARAMS);
static void print_var_decl(var_decl_t var, PARAMS);
static void print_fun_decl(fun_decl_t fun, PARAMS);
static void print_mod(mod_t mod, PARAMS);

static void print_type(type_t ty, PARAMS);
static void print_rec(rec_t rec, PARAMS);
static void print_enum(enum_t en, PARAMS);
static void print_morph_chain(morph_chain_t chain, PARAMS);

static void print_assign(assign_t assign, PARAMS);

static void print_expr(expr_t expr, PARAMS);
static void print_id_list(id_list_t id_list, PARAMS);

static void print_name(char *name, PARAMS) {
  PRINT_LIT("'%s'", name);
}

static void print_expr(expr_t expr, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_assign_kind(enum assign_kind kind, PARAMS) {
  switch (kind) {
    case SIMPLE_AS:
      PRINT_LIT("Simple '='");
      break;

    case DEEP_AS:
      PRINT_LIT("Deep Copy ':='");
      break;

    case PLUS_AS:
      PRINT_LIT("Additive '+='");
      break;

    case MINUS_AS:
      PRINT_LIT("Subtractive '-='");
      break;

    case MULT_AS:
      PRINT_LIT("Multiplicative '*='");
      break;

    case DIV_AS:
      PRINT_LIT("Divisive '/='");
      break;

    case MOD_AS:
      PRINT_LIT("Modulo '%%='");
      break;

    case OR_AS:
      PRINT_LIT("Bitwise Or '|='");
      break;

    case AND_AS:
      PRINT_LIT("Bitwise And '&='");
      break;

    case XOR_AS:
      PRINT_LIT("Bitwise Exclusive Or '^='");
      break;

    default:
      PRINT_LIT("__UNIMPLEMENTED_KIND__");
      break;
  }
}

static void print_id_list(id_list_t id_list, PARAMS){
  PRINT_NODE("Name", print_name, id_list->name);
  PRINT_NEXT("Next", print_id_list, id_list->next);
}

static void print_assign(assign_t assign, PARAMS) {
  PRINT_KIND("Kind", print_assign_kind, assign->kind);
  PRINT_LAST("Expression", print_expr, assign->expr);
}

static void print_rec(rec_t rec, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_enum(enum_t en, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_morph_chain(morph_chain_t chain, PARAMS) {
  PRINT_LIT("__TODO__");
}

static void print_type(type_t ty, PARAMS) {
  switch (ty->kind) {
    case NAME_TY:
      PRINT_LAST("Named Type", print_name, ty->u.name_ty);
      break;

    case REC_TY:
      PRINT_LAST("Record Type", print_rec, ty->u.rec_ty);
      break;

    case ENUM_TY:
      PRINT_LAST("Enum Type", print_enum, ty->u.enum_ty);
      break;

    case MORPH_TY:
      PRINT_LAST("Morph Chain Type", print_morph_chain, ty->u.morph_ty);
      break;

    default:
      PRINT_LIT("__UNIMPLEMENTED_KIND__");
      break;
  }
}

static void print_const_decl(const_decl_t con, PARAMS) {
  PRINT_NODE("Name", print_name, con->name);
  PRINT_NODE("Type", print_type, con->ty);
  PRINT_NODE("Assignment", print_assign, con->assign);
  PRINT_NEXT("Next", print_const_decl, con->next);
}

static void print_type_decl(type_decl_t ty, PARAMS) {
  PRINT_NODE("Name", print_name, ty->name);
  PRINT_LAST("Type", print_type, ty->type);
}

static void print_var_decl(var_decl_t var, PARAMS) {
  PRINT_NODE("Var", print_id_list, var->names);
  PRINT_NODE("Type", print_type, var->type);
  PRINT_NODE("Assign", print_assign, var->assign);
  PRINT_NEXT("Next", print_var_decl, var->next);
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
