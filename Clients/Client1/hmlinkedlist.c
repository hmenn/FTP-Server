#include "hmlinkedlist.h"
#include <stdlib.h>
#include <string.h>

void addList(hmlist_t *list,const hmFile_t *file){

	node_t *newNode; // yeni nod olustur
	newNode=(node_t*)malloc(sizeof(node_t));
	memset(newNode,0,sizeof(node_t));
	memset(&(newNode->filedata),0,sizeof(hmFile_t));
	newNode->filedata=*file;
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
		perror("printList null parameter");
	else{
		if(list->head==NULL){
			printf("List empty.\n");
		}
		else{
			node_t *ref = list->head;
			int i=1;
			printf("#. name size\n");
			while(ref!=NULL){
				printf("%d. %s\t%ld(byte)\n",i,ref->filedata.filename,ref->filedata.filesize);
				ref = ref->next;
				++i;
			}
			printf("Size %d\n",list->size);
		}
	}
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

	hmFile_t ser1;
	strcpy(ser1.filename,"hm0");
	ser1.filesize=15.5;
	addList(list,&ser1);
	addList(list,&ser1);
	addList(list,&ser1);
	addList(list,&ser1);
	printList(list);
	deleteList(list);
	
	free(list);
}