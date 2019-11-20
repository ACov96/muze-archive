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
	 -I $(INCLUDEDIR)

OBJLST = util.o lexer.o main.o parser.o print_tree.o codegen.o context.o \
	 symbol.o
OBJS = $(foreach obj, $(OBJLST), $(BUILDDIR)/$(obj))

TARGET = muzec

vpath %.c $(SRCDIR)
vpath %.h $(INCLUDEDIR)

include SOURCEDEPS

$(BUILDDIR) :
	mkdir -p $@

$(BUILDDIR)/%.o : %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = $(TARGET)

.PHONY : clean test

TESTSCRIPT = test.sh

test: $(TARGET)
	./$(TESTSCRIPT)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -d $(BUILDDIR)

