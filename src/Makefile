util.o: util.h util.c
	gcc -c util.c -o util.o

lexer.o: lexer.h lexer.c
	gcc -c lexer.c -o lexer.o

main.o: main.c
	gcc -c main.c -o main.o

build: util.o lexer.o main.o
	gcc -o morph util.o lexer.o main.o

clean:
	rm -f *.o morph
