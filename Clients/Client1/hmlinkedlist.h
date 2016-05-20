#ifndef _131044009_LINKED_LIST
 #define _131044009_LINKED_LIST
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_NAME 25

typedef struct{
	char filename[MAX_FILE_NAME];
	long filesize;
}hmFile_t;

typedef struct node_s{
	hmFile_t filedata;
	struct node_s *next;
}node_t;

typedef struct{
	int size;
	node_t *last; // constant O(1) time adding last
	node_t *head;
}hmlist_t;



// listin sonuna ekleme yapar
void addLast(hmlist_t *list,const hmFile_t *filedata);

// listi ekrana basar 
void printList(hmlist_t * list);

// listi temizler
void deleteList(hmlist_t *list);


#endif