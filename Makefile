CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic


all :
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell

rshell:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell

test:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
	bin/rshell < tests/redir > tests/output
