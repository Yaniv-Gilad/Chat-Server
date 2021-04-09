all: chatserver.c
	gcc chatserver.c lib.c -Wall -o server
all-GDB: chatserver.c
	gcc -g chatserver.c lib.c -Wall -o server
