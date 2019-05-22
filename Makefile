CC = gcc
CFLAGS = -g -Wall -Werror -Wno-error=unused-function

OBJS = util.o lexer.o main.o parser.o print_tree.o

TARGET = muzec

VPATH = src

include SOURCEDEPS

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.DEFAULT_GOAL = $(TARGET)

.PHONY : clean

clean:
	rm -f $(OBJS) $(TARGET)

