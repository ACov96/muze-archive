#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lexer.h"
#include "util.h"
#include "ast.h"
#include "codegen.h"
#include "morph_graph.h"
//#include "print_tree.h"

int log_enable;

struct prog_opts {
  int print_help;
  int print_tokens;
  int print_tree;
  int print_asm;
  int print_graph;
  int parse_log_enable;
  int compile_only;
  char *log_file;
  int save_asm;
  char *output_file;
  char **input_files;
};

// type_node_t *morph_graph;

struct prog_opts parse_args(int argc, char **argv) {
  struct prog_opts opts = {
    .print_help = 0,
    .print_tokens = 0,
    .print_tree = 0,
    .print_asm = 0,
    .print_graph = 0,
    .save_asm = 0,
    .compile_only = 0,
    .parse_log_enable = 0,
    .log_file = "/dev/null",
    .output_file = "a.out",
  };

  const char *opt_string = "hko:taScmpl";

  const struct option long_opts[] = {
    { "--help",   no_argument,       NULL, 'h' },
    { "--tokens", no_argument,       NULL, 'k' },
    { "--output", required_argument, NULL, 'o' },
    { "--tree",   no_argument,       NULL, 't' },
    { "--asm",    no_argument,       NULL, 'a' },
    { "--graph",  no_argument,       NULL, 'm' },
    { "--parse-log", no_argument,    NULL, 'p' },
    { "--log",    required_argument, NULL, 'l' },
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
        
    case 'S':
      opts.save_asm = 1;
      break;

    case 'c':
      opts.compile_only = 1;
      break;

    case 'm':
      opts.print_graph = 1;
      break;
    case 'p':
      opts.parse_log_enable = 1;

    case 'l':
      opts.log_file = optarg;
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
  char *linker_script_path = find_linker_script();

  struct prog_opts opts;
  
  opts = parse_args(argc, argv);

  if (!*opts.input_files) {
    fputs("Error: No input files given\n", stderr);
    exit(EXIT_FAILURE);
  }

  LOG_FILE = fopen(opts.log_file, "w");

  ll_t tokens = lex(*opts.input_files);

  if (!tokens) {
    print_errors();
    exit(EXIT_FAILURE);
  }

  if (opts.print_tokens) {
    print_tokens(tokens);
  }

  log_enable = opts.parse_log_enable;
  root_t ast_root = parse(tokens);

  if (!ast_root) {
    print_errors();
    exit(EXIT_FAILURE);
  }
	/*
  if (opts.print_tree) {
    print_tree(stdout, ast_root);
  }
	*/
  if (had_errors()) {
    print_errors();
  }

  // make graph

  type_node_t *graph = build_graph(ast_root);
  if (opts.print_graph) {
    print_graph(graph);
  }


  // type check
  if (check_types(ast_root, graph)) {
    // type checking failed
  }

  char *assembly = remove_empty_lines(codegen(ast_root, graph));
  if (opts.print_asm) {
    printf("Assembly Output:\n\n%s\n", assembly);
  }

  if (opts.save_asm) {
    FILE *out_file = fopen(opts.output_file, "w");
    fputs(assembly, out_file);
    fclose(out_file);
    return EXIT_SUCCESS;
  }

  // Fork and exec out to as+gcc to finish compilation
  int status;
  int fd[2];
  pipe(fd);
  pid_t pid = fork();
  if (pid > 0) {
    close(fd[0]);
    write(fd[1], assembly, strlen(assembly) + 1);
    close(fd[1]);
    status = 0;
    waitpid(pid, &status, 0);
  } else {
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    close(fd[1]);
    char *args[] = {"as", "-W", "-g", "-o", opts.compile_only ? opts.output_file : "a.o", "--", NULL};
    execvp(args[0], args);
  }
  if (opts.compile_only)
    return EXIT_SUCCESS;

  pid = fork();
  if (pid > 0) {
    status = 0;
    waitpid(pid, &status, 0);
  } else {
    char *args[] = {"gcc",
                    "-fno-pie",
                    "-no-pie",
                    "-g",
                    "-o",
                    opts.output_file,
                    "a.o",
                    "-lmuze",
                    "-lm",
                    "-T",
                    linker_script_path,
                    NULL};
    execvp(args[0], args);
  }
}

