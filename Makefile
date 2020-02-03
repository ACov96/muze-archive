BUILDDIR = build
SRCDIR = src
INCLUDEDIR = include

CC = gcc
CFLAGS = -g -Wall -Wfatal-errors -Werror \
	 -Wno-error=unused-function \
	 -Wno-error=unused-variable \
	 -Wno-error=unused-parameter \
	 -Wno-error=unused-value \
	 -Wno-error=unused-label \
	 -I $(INCLUDEDIR) \
	 -no-pie

OBJLST = util.o lexer.o main.o parser.o print_tree.o codegen.o context.o \
	 morph_graph.o symbol.o type.o
OBJS = $(foreach obj, $(OBJLST), $(BUILDDIR)/$(obj))

COMPILER = muzec
STDLIB = stdlib.o

vpath %.c $(SRCDIR)
vpath %.h $(INCLUDEDIR)

include SOURCEDEPS

$(BUILDDIR) :
	mkdir -p $@

$(BUILDDIR)/%.o : %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(COMPILER) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(STDLIB) : lib/stdlib.c lib/muze_stdlib.h
	$(CC) $(CFLAGS) -c -o $@ $<

all : $(COMPILER) $(STDLIB)

.DEFAULT_GOAL = all

.PHONY : clean test all

TESTSCRIPT = test.sh

test: $(COMPILER)
	./$(TESTSCRIPT)

clean:
	rm -f $(OBJS) $(COMPILER)
	rm -f a.s
	rm -d $(BUILDDIR)
