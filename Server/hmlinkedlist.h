#ifndef _131044009_LINKED_LIST
 #define _131044009_LINKED_LIST
#include <stdio.h>
#include <stdlib.h>

typedef struct{
	pid_t pidClient;
	pid_t pidServer;
	int fdClient; // two-way communicaton
}hmServer_t;

typedef struct node_s{
	hmServer_t serverData;
	struct node_s *next;
}node_t;

// koordinatlar linked list olarak tutulacak
typedef struct{
	int size;
	node_t *last; // eklemeyi constant zamana indirmek icin
	node_t *head;
}hmlist_t;



// listin sonuna ekleme yapar
void addLast(hmlist_t *list,const hmServer_t *serverData);

// listi ekrana basar 
void printList(hmlist_t * list);

// listi temizler
void deleteList(hmlist_t *list);

// delete element from list
void deleteElement(hmlist_t *list,node_t element);

// listi dosyaya basar
//void printOccurancesToLog(const char *fname,occurance_t * occ,long totalTime);


#endif