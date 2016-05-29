#ifndef _131044009_FILE_LIST_H
 #define _131044009_FILE_LIST_H


#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 25
#endif

typedef struct fileNode_s{
	char fileName[MAX_FILE_NAME];
	struct fileNode_s *next;
}fileNode_t;

typedef struct{
	fileNode_t *head;
}fileList_t;



void addFileList(fileList_t *list,const char *filename); // completed

void printFileList(fileList_t *list); //completed

void deleteAllFileList(fileList_t *list); // completed

void deleteElemFromFileList(fileList_t *list,const char *filename); //completed

void testFileList(); //completed


#endif