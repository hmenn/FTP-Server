#include "hmlinkedlist.h"
#include <stdlib.h>
#include <string.h>

void addList(hmlist_t *list,const hmServData_t *data){

	node_t *newNode; // yeni nod olustur
	newNode=(node_t*)malloc(sizeof(node_t));
	memset(newNode,0,sizeof(node_t));
	newNode->data=*data;
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
				printf("%d.Server[%ld] <-> Client[%ld] fd: %d\n",
					i,(long)ref->data.pidServer,(long)ref->data.pidClient,ref->data.fdSocket);
				ref = ref->next;
				++i;
			}
			printf("Size %d\n",list->size);
		}
	}
}

// linkedlistten belli bir elemani silecez
// hata olmasi durumunda -1 return edecektir
int deleteElementFromList(hmlist_t *list,pid_t pid){

	if(list == NULL){
		perror("deleteElementFromList : Null parameter");
		return -1;
	}

	node_t *ref =list->head;
	if(ref == NULL){
		perror("deleteElementFromList : Null list head");
		return -1;
	} 

	if(ref->data.pidServer == pid){
		list->head = ref->next;
		free(ref);
		ref=NULL;
		return 0;
	}





	printf("#### THIS IS A STUB ####\n");
	printf("PID[%ld] deleted.\n",(long)pid);

}

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

	hmServData_t ser1;
	ser1.pidClient=1595;
	ser1.pidServer=1694;
	ser1.fdSocket=5;
	addList(list,&ser1);
	addList(list,&ser1);
	addList(list,&ser1);
	addList(list,&ser1);
	printList(list);
	deleteList(list);
	
	free(list);
}