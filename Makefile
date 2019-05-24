BUILDDIR = build
SRCDIR = src
INCLUDEDIR = include

CC = gcc
CFLAGS = -g -Wall -Werror -Wno-error=unused-function -I $(INCLUDEDIR)

OBJLST = util.o lexer.o main.o parser.o print_tree.o
OBJS = $(foreach obj, $(OBJLST), $(BUILDDIR)/$(obj))

TARGET = muzec

vpath %.c $(SRCDIR)
vpath %.h $(INCLUDEDIR)

include SOURCEDEPS

$(BUILDDIR)/%.o : %.c
	mkdir -p build
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = $(TARGET)

.PHONY : clean

clean:
	rm -f $(OBJS) $(TARGET)

