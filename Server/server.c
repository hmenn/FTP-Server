#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <semaphore.h>
#include "hmlinkedlist.h"

typedef enum{LIST_SERVER,LS_CLIENT,CHECK_CLIENT,SEND_CLIENT,
	SEND_SERVER}Command_e;

#define SEM_NAME "/131044009.sm"
#define LISTEN_CONNECTION 5
#define LOG_FILE_NAME "log.hm"

	// dosya,semafor izinleri
#define PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)



static sig_atomic_t doneflag=0;
static sig_atomic_t sigflag=0;


int fdPipeToMain[2];
int fdPipeFromMain[2];
int fdSocket;

sem_t *semMain=NULL; // main server pipe fd

void sighandler(int signum){
	printf("Signal\n");
	close(fdSocket);
	doneflag=1;
	sigflag=1;
}

void startServer(int portnum);
FILE *fpLog=NULL;
hmlist_t serverList;

// initializes socket-port and returns socket fd
// coming num : # of listen to coming connetion 
int initializeSocket(int portnum,int comingnum);


void *listenPipe(void *);


// this function copied from course book
int getnamed(char *name, sem_t **sem, int val);



int main(int argc,char *argv[]){

	int iPortnum=0;

	if(argc!=2){
		fprintf(stderr,"USAGE: %s  portnumber\n",argv[0]);
		return 0;
	}

	signal(SIGINT,sighandler);
	iPortnum = atoi(argv[1]);
	fpLog = fopen(LOG_FILE_NAME,"w");
	startServer(iPortnum);

	fclose(fpLog);
	printf("Server[%ld] closed successfully.\n",(long)getpid());
	return 0;
}

// this function will start server to make some magic
void startServer(int portnum){

	struct sockaddr_in clientAddr;
	socklen_t clientlen;
	pid_t pid;
	
	// clear values
	memset(&clientAddr,0,sizeof(struct sockaddr_in));
	pid = getpid();



	 if((fdSocket=initializeSocket(portnum,LISTEN_CONNECTION))==-1){
	 	perror("Initialize socket : ");
	 	return ;
	 }
	 fprintf(fpLog,"[%ld] Socket Initialized.\n",(long)pid);


	 clientlen = sizeof(struct sockaddr_in);

	 int fdClient;
	 pid_t pidClient;
	 pid_t pidChild;
	 hmServer_t miniServer;

	 pipe(fdPipeFromMain);
	 pipe(fdPipeToMain);

	 // TODO : CHECK ERRORS

	 getnamed(SEM_NAME,&semMain,1);

	 pthread_t pipeListener;
	 pthread_create(&pipeListener,NULL,listenPipe,NULL);

	 memset(&serverList,0,sizeof(hmlist_t));
	 printf("Server started.\n");
	 while(!doneflag){
	 	
	 	printf("Waiting for clients...\n");

	 	fdClient = accept(fdSocket,(struct sockaddr*)&clientAddr,&clientlen);
	 	if(fdClient!=-1 && !doneflag){
	 		read(fdClient,&pidClient,sizeof(pid_t));

	 		if((pidChild=fork())==-1){
	 			perror("Fork failed : ");
	 			doneflag=1;
	 		}else if(pidChild==0){ // child here
	 			sem_close(semMain);
	 			deleteList(&serverList);
	 			break;
	 		}else{//parent here
	 			memset(&miniServer,0,sizeof(hmServer_t));
		 		miniServer.pidClient=pidClient;
		 		miniServer.fdClient=fdClient;
	 			miniServer.pidServer = pidChild;
	 			addLast(&serverList,&miniServer);
	 			printf("Client[%ld] connected to Server[%ld].\n",
	 									(long)pidClient,(long)pidChild);
	 		}

	 	}else doneflag=1;
	 }

	 if(pidChild==0){ // child continue

		 // listen incoming request
		 int  command;
		 //close(fdPipeToMain[0]);
		 //close(fdPipeFromMain[1]);
		 while(!doneflag){
		 	command=-1;
		 	int err = read(fdClient,&command,sizeof(int));
		 	if(err==0){
		 		doneflag=1;
		 	}

		 	switch(command){

		 		case LS_CLIENT:{
		 			printf("Client want to list connedted clients pids...\n");
		 			int request = LS_CLIENT;

		 			write(fdPipeToMain[1],&request,sizeof(int));
		
		 			while(1){
		 				pid_t cl;
		 				read(fdPipeFromMain[0],&cl,sizeof(pid_t));
		 				write(fdClient,&cl,sizeof(pid_t));
		 				if(cl==-1)
		 					break;
		 			}
		 		} break;

		 	}
		 	printf("Command completed\n");
		 }

	 }else{
	 	//printList(&serverList);
	 	deleteList(&serverList);
	 	pid_t endpid=-1;
	 	write(fdPipeToMain[1],&endpid,sizeof(pid_t));
	 }


	 pthread_join(pipeListener,NULL);
	 close(fdSocket);
	 sem_close(semMain);
	 unlink(SEM_NAME);
}


void *listenPipe(void *args){

	//close(fdPipeFromMain[0]);
	//close(fdPipeToMain[1]);

	int command=-1;
	while(!doneflag){

		if(read(fdPipeToMain[0],&command,sizeof(int))>0){
		
			if(command==LS_CLIENT){ // send connecting client pids
				node_t *ref = serverList.head;
				while(ref!=NULL){
					pid_t client = ref->serverData.pidClient;
					write(fdPipeFromMain[1],&client,sizeof(pid_t));
					ref=ref->next;
				}
				pid_t endpid =-1;
				write(fdPipeFromMain[1],&endpid,sizeof(pid_t));
			}
		}

	}


}



// listennum : listen to the incoming connections
// return socket file descriptor
int initializeSocket(int portnum,int comingnum){

	struct sockaddr_in serverAddr;
	socklen_t serverlen;
	int fdSocket=-1;
	// clear server adress
	memset(&serverAddr,0,sizeof(struct sockaddr_in));

	if((fdSocket = socket(AF_INET,SOCK_STREAM,0))<0){
		perror("Socket initialize : ");
		return -1;
	}

	// IPv4
	serverAddr.sin_family = AF_INET;
	// all local ip addresses
	// convert ip address number to network bytes
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY); // host to network long integer
	// initialize port number
	// convert short integer port num to network bytes
	serverAddr.sin_port = htons(portnum);// host to network short integer

	serverlen = sizeof(struct sockaddr_in);

	if(bind(fdSocket,(struct sockaddr*)&serverAddr,serverlen)<0){
		perror("Binding socket : ");
		return -1;
	}

	// listen socket comming connetions
	if(listen(fdSocket,comingnum)){
		perror("Listen socket : ");
		return -1;
	}

	return fdSocket;
}


int getnamed(char *name, sem_t **sem, int val) {
	while (((*sem = sem_open(name, FLAGS, PERMS, val)) == SEM_FAILED) &&
									(errno == EINTR)) ;
	if (*sem != SEM_FAILED)
		return 0;
	if (errno != EEXIST)
		return -1;
	while (((*sem = sem_open(name, 0)) == SEM_FAILED) && (errno == EINTR)) ;

	if (*sem != SEM_FAILED)
		return 0;

	return -1;
}