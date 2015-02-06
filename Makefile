CC=g++
CFLAGS=-g -Wall -Werror -ansi -pedantic


all : rshell ls

rshell:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell

ls:
	mkdir -p bin
	$(CC) $(CFLAGS) src/ls.cpp -o bin/ls


rshelltest:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
	bin/rshell < tests/redir > tests/output
	less tests/output

rshellmanualtest:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
	clear && bin/rshell
