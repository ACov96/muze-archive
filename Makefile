BUILDDIR = build
SRCDIR = src
INCLUDEDIR = include
LIBDIR = lib

CC = gcc
CFLAGS = -g -Wall -Wfatal-errors -Werror \
	 -Wno-error=unused-function \
	 -Wno-error=unused-variable \
	 -Wno-error=unused-parameter \
	 -Wno-error=unused-value \
	 -Wno-error=unused-label \
	 -I $(INCLUDEDIR) \
	 -no-pie

OBJLST = util.o lexer.o main.o parser.o print_tree.o codegen.o context.o morph_graph.o symbol.o
OBJS = $(foreach obj, $(OBJLST), $(BUILDDIR)/$(obj))

COMPILER = muzec

vpath %.c $(SRCDIR)
vpath %.h $(INCLUDEDIR)

include SOURCEDEPS

$(BUILDDIR) :
	mkdir -p $@

$(BUILDDIR)/%.o : %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

lib/libs.o :
	cd $(LIBDIR) && make

$(COMPILER) : $(OBJS) | lib/libs.o
	$(CC) $(CFLAGS) -o $@ $^

all : $(COMPILER) $(STDLIB)

.DEFAULT_GOAL = all

.PHONY : clean test all

TESTSCRIPT = test.sh

test: $(COMPILER)
	./$(TESTSCRIPT)

clean:
	rm -f $(OBJS) $(COMPILER)
	rm -f a.s
	rm -rf $(BUILDDIR)
	cd lib && make clean
