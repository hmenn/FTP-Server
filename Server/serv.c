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
#include "serv.h"
#include "hmlinkedlist.h"

int gI_serverPortNum=-1; // port number -- global integer
pid_t gPid_server=0; // global server pid
int gI_socketFd=-1;
sem_t *gSemP_pipeSem;
hmlist_t serverList;

int fdClient=-1;// client socket fd

int gPipe_crpw[2]; // 'C'hild 'R'ead 'P'arent 'W'rite global pipe
int gPipe_cwpr[2]; // 'C'hild 'W'rite 'P'arent 'R'ead  global pipe


static sig_atomic_t doneflag=0;

int main(int argc,char *argv[]){

	if(argc != 2){
		fprintf(stderr, "USAGE : %s portnumber\n",argv[0]);
		return 1;
	}

	signal(SIGINT,sighandler);
	signal(SIGPIPE,sigPipeHandler);

	gPid_server = getpid();
	gI_serverPortNum = atoi(argv[1]);
	startServer(gI_serverPortNum);

	printf("[%ld] server closed\n",(long)gPid_server);

	return 0;
}

void sighandler(int signum){
	printf("%s handled\n",strsignal(signum));
	doneflag=1;
	close(gI_socketFd); // interrupt accept socket
	//close(fdClient);
	/*close(gPipe_cwpr[0]); // close read pipe to kill listener thread
	close(gPipe_cwpr[1]);*/
}

void sigPipeHandler(int signum){
	printf("%s -handled\n",strsignal(signum));
}


void startServer(int portnum){


	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;

	// initialize size of struct
	clientAddrLen = sizeof(struct sockaddr_in);

	//clear junk values
	memset(&clientAddr,0,clientAddrLen);

	if((gI_socketFd = initializeSocket(portnum,LISTEN_NUMBER))==-1){
		perror("Initialize socket");
		return;
	}

	fprintf(stderr,"[%ld] initialized socket.\n",(long)gPid_server);

	getnamed(PIPE_CONTROL_SEM,&gSemP_pipeSem,1); // create semafor for control pipe collisions
	pipe(gPipe_cwpr);
	pipe(gPipe_crpw);
	// TODO : check pipe errors



	pthread_t th_pipeListener;
	pthread_create(&th_pipeListener,NULL,listenPipe,NULL);


	memset(&serverList,0,sizeof(hmlist_t));
	printf("[%ld] server started.\n",(long)gPid_server);


	hmServData_t miniServer;
	

	
	pid_t pidChild=-1;
	pid_t pidClient=-1;

	while(!doneflag){

		printf("[%ld]MainServer waiting for clients...\n",(long)gPid_server);
		// accept clients
		fdClient = accept(gI_socketFd,(struct sockaddr*)&clientAddr,&clientAddrLen);
		if(fdClient !=-1 && !doneflag){

			// client send pid address and server creates new mini server to serv
			read(fdClient,&pidClient,sizeof(pid_t));

			if( (pidChild = fork())==-1){
				perror("Fork");
				doneflag=1;
			}else if(pidChild==0){ // child here
				break;
			}else{ // parent here
				// pair server-client informations
				memset(&miniServer,0,sizeof(hmServData_t));
				miniServer.pidServer=pidChild;
				miniServer.pidClient=pidClient;
				miniServer.fdSocket = fdClient;
				addList(&serverList,&miniServer);
				printf("Client[%ld] connected to server[%ld].\n",
	 									(long)pidClient,(long)pidChild);
			}
		}else doneflag=1;

	}


	if(pidChild ==0){ // child here
		/*sem_close(gSemP_pipeSem);
		getnamed(PIPE_CONTROL_SEM,&gSemP_pipeSem,1);*/
		gPid_server = getpid();
		
		startMiniServer(pidClient);
		pthread_join(th_pipeListener,NULL);
		sem_close(gSemP_pipeSem);
		close(gI_socketFd);
		
	}else{ // main server here

		killAllChilds(SIGINT);
		close(gPipe_cwpr[1]); //close pipe to kill thread

		while(wait(NULL)!=-1){

		}

		deleteList(&serverList);
		printf("Waiting for thread\n");
		pthread_join(th_pipeListener,NULL);
		sem_close(gSemP_pipeSem);
		close(gI_socketFd);
	}
}


void startMiniServer(pid_t pidClient){

	Command_e command=DIE;
	//send pid address to client
	write(fdClient,&gPid_server,sizeof(pid_t));
	while(!doneflag){

		read(fdClient,&command,sizeof(Command_e));
		printf("[%ld]MiniServer read command:%d\n",(long)gPid_server,command);
		if(!doneflag){
			if(command==LS_CLIENT){

				write(gPipe_cwpr[1],&command,sizeof(Command_e));
				write(fdClient,&command,sizeof(Command_e)); // say client, which command result will send
				int clientnum;
				read(gPipe_crpw[0],&clientnum,sizeof(int));
				write(fdClient,&clientnum,sizeof(int)); // push client number to socket
				int i=0;
				pid_t pidTemp=-1;
				for(i=0; i!=clientnum;++i){ // read pid from pipe and write to socket
					read(gPipe_crpw[0],&pidTemp,sizeof(pid_t));
					write(fdClient,&pidTemp,sizeof(pid_t));
				}
			}else if(command ==DIE){
				printf("[%ld]MiniServer read die command from [%ld]client\n",
					(long)gPid_server,(long)pidClient);
				write(fdClient,&command,sizeof(Command_e));

				doneflag=1;
				// TODO: CHANGE DONEFLAG WITH SOMETHING DIFF

			}
		}
	}





}






void *listenPipe(void *args){

	Command_e command;
	// take requests from pipe
	while(!doneflag && read(gPipe_cwpr[0],&command,sizeof(Command_e))>0 && !doneflag){
		printf("[%ld]MainServer read command %d\n",(long)gPid_server,command);
		if(command == LS_CLIENT){
			// first send number of clients
			write(gPipe_crpw[1],&(serverList.size),sizeof(int));
			node_t *node=serverList.head;
			while(!doneflag && node!=NULL){ //send client pids
				write(gPipe_crpw[1],&(node->data.pidClient),sizeof(pid_t));
				node=node->next;
			}
		}
	}
	return NULL;
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
	while (((*sem = sem_open(name, FLAGS, 0600, val)) == SEM_FAILED) &&
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

void killAllChilds(int signum){
	node_t *head = serverList.head;
	pid_t pidMiniServer;
	Command_e command=DIE;

	while(head!=NULL){
		pidMiniServer = head->data.pidServer;
		//write(head->data.fdSocket,&command,sizeof(command));
		kill(pidMiniServer,signum);
		close(head->data.fdSocket);
		printf("[%ld]MainServer send kill(%d) to [%ld]Child\n",
						(long)gPid_server,signum,(long)pidMiniServer);
		head = head->next;
	 }
}