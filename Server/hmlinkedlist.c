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
void deleteElemFromList(hmlist_t *list,pid_t pidServer){

	if(list == NULL){
		perror("deleteElemFromList null parameter");
		exit(1);
	}

	if(list->head ==NULL){
		return ;
	}

	if(list->head->data.pidServer==pidServer){
		node_t *temp = list->head;
		list->head = list->head->next;
		free(temp);
		temp=NULL;
	}else{
		node_t *curr = list->head;
		node_t *temp=NULL;
		while(curr->next!=NULL){
			if(curr->next->data.pidServer==pidServer){
				temp = curr ->next;
				curr->next =temp->next;
				break;
			}
			curr = curr->next;
		}

		free(temp);
		temp=NULL;
	}
	list->size -=1; 
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