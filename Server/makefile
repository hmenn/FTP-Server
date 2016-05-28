all:
	gcc hmlinkedlist.c -c
	gcc serv.c -c -DDEBUG
	gcc hmlinkedlist.o serv.o -o serv -pthread
	rm -rf *.o

v1:
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./serv 9415

v3:
	valgrind --tool=massif ./serv 9415
v2:
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./serv 9416
