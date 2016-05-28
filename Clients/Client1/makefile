

all:
	gcc cl.c -c
	gcc cl.o -o client -pthread
	rm -rf *.o
v1:
	valgrind --show-leak-kinds=all --track-origins=yes --leak-check=full ./client 127.45.65.1 9415
v2:
	valgrind --show-leak-kinds=all --track-origins=yes --leak-check=full ./client 127.32.54.95 9416
