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


//#define DEBUG 0
#define MAX_LINE_LEN 80
#define MAX_FILE_NAME 25
#define INVALID_FILE_NAME "/"
#define LOCAL_DIR "./"


typedef enum COMMAND{
	LIST_SERVER,LS_CLIENT,SEND_FILE,DIE,CHECK_CLIENT
}Command_e;


void sigHandler(int signum);

// show manual page
void showHelpManual();

// list online client pids
void lsClient();

int connectServer(const char *ipnum,int portnum);

void sigAlarmHandler(int signum);

void *socketListener(void *args);

int listFilesInDir(DIR *dir);



void listServer();

long isFileInDir(DIR *dir,const char *fileName);
int sendFile(DIR* dir, const char *fileName,pid_t pid);

#endif