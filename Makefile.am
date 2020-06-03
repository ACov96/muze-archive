bin_PROGRAMS = muzec
lib_LIBRARIES = libmuze.a

AM_CPPFLAGS = -I${INCLUDE_DIR} -DPREFIX="\"$(prefix)\""

# muzec
muzec_SOURCES = \
src/codegen.c \
src/context.c \
src/lexer.c \
src/main.c \
src/morph_graph.c \
src/parser.c \
src/print_tree.c \
src/symbol.c \
src/type.c \
src/util.c

# libmuze
libmuze_a_SOURCES = \
src/morph_graph.c \
src/muze_stdlib.c


# Custom linker script
sharedir = $(prefix)/share/muze
share_DATA = share/muze/muze.ld

# Testing
TESTS = test.sh
