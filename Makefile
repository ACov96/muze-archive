CC = gcc
CFLAGS = -g -Wall -Werror -Wno-error=unused-function

OBJS = util.o lexer.o main.o parser.o print_tree.o

VPATH = src

include SOURCEDEPS

morph : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = morph

.PHONY : clean

clean:
	rm -f $(OBJS) morph

