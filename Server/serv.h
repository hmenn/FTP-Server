#ifndef _131044009_SERVER_H
#define _131044009_SERVER_H


#define FLAGS (O_CREAT | O_EXCL)
#define LISTEN_NUMBER 5
#define PIPE_CONTROL_SEM "/ppsem.hm"


typedef enum COMMAND{
	LIST_LOCAL,LIST_SERVER,LS_CLIENT,SEND_FILE,DIE
}Command_e;


void startMiniServer(pid_t pidClient);
void startServer(int portnum);
int initializeSocket(int portnum,int comingnum);
int getnamed(char *name, sem_t **sem, int val);
void *listenPipe(void *args);
void sighandler(int signum);
void sigPipeHandler(int signum);
void killAllChilds();

#endif