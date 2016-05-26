#ifndef _131044009_CLIENT_H
#define _131044009_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "hmlinkedlist.h"


#define DEBUG 0
#define MAX_LINE_LEN 80

typedef enum COMMAND{
	LIST_LOCAL,LIST_SERVER,LS_CLIENT,SEND_FILE,DIE
}Command_e;


void sigHandler(int signum);

void showHelpManual();

int connectServer(const char *ipnum,int portnum);

void sigAlarmHandler(int signum);

hmlist_t* readDirectory(const char *dirpath);

void *socketListener(void *args);

int isRegFile(const char * fileName);




#endif