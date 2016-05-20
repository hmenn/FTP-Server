#include "hmlinkedlist.h"
#include <stdlib.h>
#include <string.h>

void addLast(hmlist_t *list,const hmServer_t *serverData){

	node_t *newNode; // yeni nod olustur
	newNode=(node_t*)malloc(sizeof(node_t));
	memset(newNode,0,sizeof(node_t));
	newNode->serverData=*serverData;
	newNode->next=NULL;

	if(list->head==NULL){ // bos ise basina ekle
		list->size=0;
		list->head = newNode;
		list->last = newNode;
	}else{ // degilse sonuna ekle
		list->last->next = newNode;
		list->last = list->last->next;

	}
	list->size +=1;
}


// tum elemanlari sirayla dosyaya basicak fonksiyon
void printList(hmlist_t * list){

	if(list ==NULL)
		perror("Null Pointer Parameter : ");
	else{
		if(list->head==NULL){
			printf("List empty.\n");
		}
		else{
			node_t *ref = list->head;
			int i=1;
			while(ref!=NULL){
				printf("%d.Server[%ld] <-> Client[%ld] fd: %d\n",i,(long)ref->serverData.pidServer,(long)ref->serverData.pidClient,ref->serverData.fdClient);
				ref = ref->next;
				++i;
			}
			printf("Size %d\n",list->size);
		}
	}
}
/*
// listi dosyaya basar
// list icinde her dosyanin adi koordinatlari vardir.
// islemin toplam zamaninida ekrana basar
void printOccurancesToLog(const char *fname,occurance_t * occ,long totalTime){

	FILE * fpLog = fopen(fname,"a");
	int i=1;

	if(occ ==NULL)
		perror("Null parameter");
	else{
		if(occ->head==NULL)
			fprintf(fpLog,"empty list\n");
		else{
			fprintf(fpLog, "\n#####################################\n");
			fprintf(fpLog, "\nFile : %s\n",occ->fileName);
			fprintf(fpLog, "Total Time : %ld(ms)\n\n",totalTime);
			node_t *ref = occ->head;
			while(ref!=NULL){
				fprintf(fpLog,"%d. row: %d - column: %d\n",i++,ref->row,ref->column);
				ref = ref->next;
			}
			fprintf(fpLog,"File Total %d\n",occ->total);
		}
	}
	fflush(fpLog); // log flusing
	fclose(fpLog);
}*/

// linked listi siler
void deleteList(hmlist_t *list){
	if(list!=NULL){
		if(list->head !=NULL){
			node_t *ref;
			while(list->head != NULL){
				ref = list->head;
				list->head = ref->next;
				free(ref);
			}
		}
	}
}


// link list test
int test(){

	hmlist_t *list = malloc(sizeof(hmlist_t));

	list->size=0;
	list->head=NULL;
	list->last=NULL;

	hmServer_t ser1;
	ser1.pidClient=1595;
	ser1.pidServer=1694;
	ser1.fdClient=5;
	addLast(list,&ser1);
	addLast(list,&ser1);
	addLast(list,&ser1);
	addLast(list,&ser1);
	printList(list);
	deleteList(list);
	
	free(list);
}