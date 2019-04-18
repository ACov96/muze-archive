CC = gcc
CFLAGS = -g -Wall

OBJS = util.o lexer.o main.o

VPATH = src

util.o   : util.h
lexer.o  : lexer.h util.h

morph : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = morph

.PHONY : clean

clean:
	rm -f $(OBJS) morph

