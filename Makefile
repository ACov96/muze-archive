CC = gcc
CFLAGS = -g -Wall

OBJS = util.o lexer.o main.o

VPATH = src

include SOURCEDEPS

morph : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = morph

.PHONY : clean

clean:
	rm -f $(OBJS) morph

