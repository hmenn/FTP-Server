#include "fileList.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void addFileList(fileList_t *list,const char *fileName){

	if(list == NULL){
		perror("Null paremeter in addFileList");
		return;
	}else{
		fileNode_t *newFile=NULL;
		newFile = malloc(sizeof(fileNode_t));
		memset(newFile,0,sizeof(fileNode_t));

		newFile->next=NULL;
		strcpy(newFile->fileName,fileName);

		if(list->head==NULL){
			list->head = newFile;
		}else{
			fileNode_t *curr = list->head;
			while(curr->next!=NULL){
				curr = curr->next;
			}
			curr->next=newFile;
		}
	}
}

// tum elemanlari sirayla dosyaya basicak fonksiyon
void printFileList(fileList_t * list){

	if(list ==NULL)
		perror("Null Pointer Parameter : ");
	else{
		fileNode_t *ref = list->head;
		printf("USED FILES\n");
		while(ref!=NULL){
			printf("%s\n",ref->fileName);
			ref = ref->next;
		}
	}
}

void deleteElemFromFileList(fileList_t *list,const char *fileName){


	if(list == NULL){
		perror("deleteElemFromFileList null parameter");
		exit(1);
	}

	if(list->head ==NULL){
		return ;
	}

	if(strcmp(list->head->fileName,fileName)==0){
		fileNode_t *temp = list->head;
		list->head = list->head->next;
		free(temp);
		temp=NULL;
	}else{
		fileNode_t *curr = list->head;
		fileNode_t *temp=NULL;
		while(curr->next!=NULL){
			if(strcmp(curr->next->fileName,fileName)==0){
				temp = curr ->next;
				curr->next =temp->next;
				break;
			}
			curr = curr->next;
		}

		free(temp);
		temp=NULL;
	}
}

// linked listi siler
void deleteAllFileList(fileList_t *list){
	if(list!=NULL){
		if(list->head !=NULL){
			fileNode_t *ref;
			while(list->head != NULL){
				ref = list->head;
				list->head = ref->next;
				free(ref);
			}
		}
	}
}

// link list test
void testFileList(){

	fileList_t list;
	list.head=NULL;

	addFileList(&list,"hsn1");
	addFileList(&list,"hsn");
	addFileList(&list,"hm");
	addFileList(&list,"hm");
	
	printFileList(&list);
	deleteElemFromFileList(&list,"hm");
	deleteElemFromFileList(&list,"hsn1");
	printFileList(&list);
	deleteAllFileList(&list);
	
	
		
	//free(list);
}