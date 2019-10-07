#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "lexer.h"
#include "util.h"
#include "ast.h"
#include "codegen.h"
#include "print_tree.h"

struct prog_opts {
  int print_help;
  int print_tokens;
  int print_tree;
  int print_asm;
  char *output_file;
  char **input_files;
};

struct prog_opts parse_args(int argc, char **argv) {
  struct prog_opts opts = {
    .print_help = 0,
    .print_tokens = 0,
    .print_tree = 0,
    .print_asm = 0,
    .output_file = "a.out",
  };

  const char *opt_string = "hko:t";
  const struct option long_opts[] = {
    { "--help",   no_argument,       NULL, 'h' },
    { "--tokens", no_argument,       NULL, 'k' },
    { "--output", required_argument, NULL, 'o' },
    { "--tree",   no_argument,       NULL, 't' },
    { "--asm",    no_argument,       NULL, 'a' }
  };

  for (int opt = getopt_long(argc, argv, opt_string, long_opts, NULL);
       opt != -1;
       opt = getopt_long(argc, argv, opt_string, long_opts, NULL)) {
    switch (opt) {
      case 'h':
        opts.print_help = 1;
        break;

      case 'o':
        opts.output_file = optarg;
        break;

      case 'k':
        opts.print_tokens = 1;
        break;

      case 't':
        opts.print_tree = 1;
        break;

      case 'a':
        opts.print_asm = 1;
        break;
        
      // Error cases
      case '?':
        break;

      case ':':
        break;

      default:
        break;
    }
  }

  opts.input_files = argv + optind;

  return opts;
}

int main(int argc, char* argv[]) {
  struct prog_opts opts;
  
  opts = parse_args(argc, argv);

  if (!*opts.input_files) {
    fputs("Error: No input files given\n", stderr);
    exit(EXIT_FAILURE);
  }

  ll_t tokens = lex(*opts.input_files);

  if (!tokens) {
    print_errors();
    exit(EXIT_FAILURE);
  }

  if (opts.print_tokens) {
    print_tokens(tokens);
  }

  root_t ast_root = parse(tokens);

  if (!ast_root) {
    print_errors();
    exit(EXIT_FAILURE);
  }

  if (opts.print_tree) {
    print_tree(stdout, ast_root);
  }

  if (had_errors()) {
    print_errors();
  }

  char *assembly = codegen(ast_root);
  if (opts.print_asm) {
    printf("Assembly Output:\n%s\n", assembly);
  }
  
}

