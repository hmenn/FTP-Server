

all:
	gcc hmlinkedlist.c -c
	gcc cl.c -c
	gcc hmlinkedlist.o cl.o -o client -pthread
	rm -rf *.o