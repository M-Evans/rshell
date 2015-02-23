CC=g++
CFLAGS=-g -Wall -Werror -ansi -pedantic --std=c++11


all : rshell ls cp

rshell:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell

ls:
	mkdir -p bin
	$(CC) $(CFLAGS) src/ls.cpp -o bin/ls

cp:
	mkdir -p bin
	$(CC) $(CFLAGS) src/cp.cpp -o bin/cp

rshelltest:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
	bin/rshell < tests/redir > tests/output
	less tests/output

rshellmanualtest:
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.cpp -o bin/rshell
	clear && bin/rshell
