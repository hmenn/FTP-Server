#ifndef _131044009_SERVER_H
#define _131044009_SERVER_H


#define FLAGS (O_CREAT | O_EXCL)
#define LISTEN_NUMBER 5
#define PIPE_CONTROL_SEM "/ppsem.hm" // semafor to control main server pipes
#define MAX_FILE_NAME 25
#define INVALID_FILE_NAME "/" // specify end of sending file names
#define LOCAL_DIR "./"
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)



typedef enum {
	LIST_SERVER,LS_CLIENT,SEND_FILE,DIE,CHECK_CLIENT
}Command_e; // commands to communicate between client-server

// Start new mini server and initialize values
void startMiniServer(pid_t pidClient);

// starts mian server
void startServer(int portnum);

// initialize socket
int initializeSocket(int portnum);

// creates named semaphore
int getnamed(char *name, sem_t **sem, int val);

// Main server will create listener thread to communicate between
// Main-mini server with pipe, this thread will use this function
void *listenPipe(void *args);

// signal handler
void sighandler(int signum);

// end of program kill all childs
void killAllChilds();


// fdClient : Client socked fildes
// pidClient : serving which client -> use in printf
void lsClient(int fdClient,pid_t pidClient);


// reads local files and writes to fd
// fd can be socket,pipe,or stdout
int listLocalFiles(DIR *dir,int fd);


// return differecens between two time in milliseconds
long getTimeDif(struct timeval start, struct timeval end);



pid_t getClientServerPid(pid_t pidClient);

void sendFile(pid_t whosent);


void *fifoController(void *args);


#endif