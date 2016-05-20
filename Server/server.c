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


#define LISTEN_CONNECTION 5
#define LOG_FILE_NAME "log.hm"



static sig_atomic_t doneflag=0;

void sighandler(int signum){
	printf("Signal\n");
	doneflag=1;
}

void startServer(int portnum);
FILE *fpLog=NULL;

// initializes socket-port and returns socket fd
// coming num : # of listen to coming connetion 
int initializeSocket(int portnum,int comingnum);

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

	return 0;
}

// this function will start server to make some magic
void startServer(int portnum){

	struct sockaddr_in clientAddr;
	socklen_t clientlen;
	int fdSocket;
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
	 while(!doneflag){

	 	printf("Server started.\n");
	 	printf("Waiting for clients...\n");

	 	fdClient = accept(fdSocket,(struct sockaddr*)&clientAddr,&clientlen);
	 	if(fdClient!=-1 && !doneflag){
	 		read(fdClient,&pidClient,sizeof(pid_t));
	 		printf("[%ld] client connected to server.\n",(long)pidClient);



	 	}else doneflag=1;
	 }

	 printf("asd\n");
	 close(fdSocket);
	 close(fdClient);

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