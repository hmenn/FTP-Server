

all:
	gcc hmlinkedlist.c -c
	gcc client.c -c
	gcc hmlinkedlist.o client.o -o client -pthread
	rm -rf *.o