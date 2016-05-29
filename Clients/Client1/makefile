

all:
	gcc cl.c -c
	gcc cl.o -o client -pthread
	rm -rf *.o
v1: all
	valgrind --show-leak-kinds=all --track-origins=yes --leak-check=full ./client 127.45.65.1 9415
v2:
	valgrind --show-leak-kinds=all --track-origins=yes --leak-check=full ./client 127.32.54.95 9416
run1:
	./client 127.51.64.15 9415
run2:
	./client 127.53.64.61 9416
