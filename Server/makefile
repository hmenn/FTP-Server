

all:
	gcc -c hmlinkedlist.c
	gcc -c server.c
	gcc hmlinkedlist.o server.o -o server -pthread
	rm -rf *.o