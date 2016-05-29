#ifndef _131044009_LINKED_LIST
 #define _131044009_LINKED_LIST
#include <stdio.h>
#include <stdlib.h>

typedef struct{
	pid_t pidClient;
	pid_t pidServer;
	int fdSocket; // two-way communicaton
}hmServData_t;

typedef struct node_s{
	hmServData_t data;
	struct node_s *next;
}node_t;

// koordinatlar linked list olarak tutulacak
typedef struct{
	int size;
	node_t *last; // constant time adding O(1)
	node_t *head;
}hmlist_t;



// listin sonuna ekleme yapar
void addList(hmlist_t *list,const hmServData_t *data);

// listi ekrana basar 
void printList(hmlist_t * list);

// listi temizler
void deleteList(hmlist_t *list);

// delete element from list
void deleteElemFromList(hmlist_t *list,pid_t pid);

// listi dosyaya basar
//void printOccurancesToLog(const char *fname,occurance_t * occ,long totalTime);


#endif