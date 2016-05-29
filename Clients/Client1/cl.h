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
#define MAX_LINE_LEN 80 // max line length to read from stdin
#define MAX_FILE_NAME 25 // max file name length
#define INVALID_FILE_NAME "/" // to specify server stop send file list
#define LOCAL_DIR "./" // local directory path


typedef enum{
	LIST_SERVER,LS_CLIENT,SEND_FILE,DIE,CHECK_CLIENT
}Command_e; // command list to communicate between server and client

// JUST HANDLED SIGINT
// signal handler
void sigHandler(int signum);

// show manual page
void showHelpManual();

// list online client pids
void lsClient();

// send request server and list files in server
void listServer(pid_t serverpid);

// start connection to server
int connectServer(const char *ipnum,int portnum);

// sig alarm handler
// if server can not connect server in 2sec, will raise sig alarm
void sigAlarmHandler(int signum);

// socket listener thread will listen socket and accept datas then will direct
void *socketListener(void *args);

// list all files in directory
int listFilesInDir(DIR *dir);

// check file is in directory or nor
long isFileInDir(DIR *dir,const char *fileName);

// sends file between client-client or client-server
// if pid equal 0 or invalid, sends server
int sendFile(DIR* dir, const char *fileName,pid_t pid);
void sendFileWrapper(DIR *dir,char *line); // send file wrapper

#endif