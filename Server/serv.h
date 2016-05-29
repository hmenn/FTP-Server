#ifndef _131044009_SERVER_H
#define _131044009_SERVER_H


#define FLAGS (O_CREAT | O_EXCL)
#define LISTEN_NUMBER 5
#define PIPE_CONTROL_SEM "/ppsem.hm"
#define MAX_FILE_NAME 25
#define INVALID_FILE_NAME "/"
#define LOCAL_DIR "./"
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)



typedef enum COMMAND{
	LIST_SERVER,LS_CLIENT,SEND_FILE,DIE,CHECK_CLIENT
}Command_e;


void startMiniServer(pid_t pidClient);
void startServer(int portnum);
int initializeSocket(int portnum,int comingnum);
int getnamed(char *name, sem_t **sem, int val);
void *listenPipe(void *args);
void sighandler(int signum);
void killAllChilds();


// fdClient : Client socked fildes
// pidClient : serving which client -> use in printf
void lsClient(int fdClient,pid_t pidClient);


// reads local files and writes to fd
// fd can be socket,pipe,or stdout
int listLocalFiles(DIR *dir,int fd);
int isRegFile(const char * fileName);



pid_t getClientServerPid(pid_t pidClient);

void sendFile();


void *fifoController(void *args);


#endif