CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic


all :
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell

rshell:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
