#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> // DIR - dirent
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <semaphore.h>
#include "serv.h"
#include "hmlinkedlist.h"
#include "fileList.h"

int gI_serverPortNum=-1; // port number -- global integer
pid_t gPid_server=0; // global server pid
int gI_socketFd=-1;
sem_t *gSemP_pipeSem;
sem_t *gSemP_fifoSem;
sem_t *gSemP_serverSem;
hmlist_t serverList;
pthread_mutex_t gMutex_lockSocket;

int fdClient=-1;// client socket fd

int gPipe_crpw[2]; // 'C'hild 'R'ead 'P'arent 'W'rite global pipe
int gPipe_cwpr[2]; // 'C'hild 'W'rite 'P'arent 'R'ead  global pipe


static sig_atomic_t doneflag=0;

int main(int argc,char *argv[]){


	//testFileList();
	if(argc != 2){
		fprintf(stderr, "USAGE : %s portnumber\n",argv[0]);
		return 1;
	}

	signal(SIGINT,sighandler);

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
	close(fdClient);
	/*close(gPipe_cwpr[0]); // close read pipe to kill listener thread
	close(gPipe_cwpr[1]);*/
}

void startServer(int portnum){

	struct sockaddr_in clientAddr; // client socket informations
	socklen_t clientAddrLen; // client socket address length

	// initialize size of struct
	clientAddrLen = sizeof(struct sockaddr_in);

	//clear junk values
	memset(&clientAddr,0,clientAddrLen);

	if((gI_socketFd = initializeSocket(portnum,LISTEN_NUMBER))==-1){
		perror("Initialize socket");
		return;
	}

	fprintf(stderr,"[%ld] initialized socket.\n",(long)gPid_server);

	// create a semafor to control writing sequential to pipe
	sem_unlink(PIPE_CONTROL_SEM);
	getnamed(PIPE_CONTROL_SEM,&gSemP_pipeSem,1); 

	pipe(gPipe_cwpr);
	pipe(gPipe_crpw);
	// TODO : check pipe errors


	pthread_t th_pipeListener;
	pthread_create(&th_pipeListener,NULL,listenPipe,NULL);

	memset(&serverList,0,sizeof(hmlist_t)); //clear junks
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
				addList(&serverList,&miniServer); // store child informations
				printf("Client[%ld] connected to server[%ld].\n",
	 									(long)pidClient,(long)pidChild);
			}
		}else doneflag=1;

	}

	if(pidChild ==0){ // child here
		// no need to re-open semafor because we forked all memory
		/*sem_close(gSemP_pipeSem);
		getnamed(PIPE_CONTROL_SEM,&gSemP_pipeSem,1);*/
		gPid_server = getpid(); // assing new mini server pid
		deleteList(&serverList); // remove parent values
		startMiniServer(pidClient); // start server
		pthread_join(th_pipeListener,NULL);
		sem_close(gSemP_pipeSem);
		close(gI_socketFd); // close spesific socket fildes
		// return main and close mini server
	}else{ // main server here

		killAllChilds(SIGINT); 
		close(gPipe_cwpr[1]); //close pipe to kill thread

		while(wait(NULL)!=-1){} // wait childs
		// TODO : create thread to wait childs

		deleteList(&serverList);
		printf("[%ld]MainServer Waiting for threads\n",(long)gPid_server);
		pthread_join(th_pipeListener,NULL);
		sem_close(gSemP_pipeSem);
		close(gI_socketFd);
	}
}


void startMiniServer(pid_t pidClient){

	Command_e command=DIE;
	DIR *pDir=NULL;
	char strSemNameFifo[MAX_FILE_NAME];
	pthread_t th_fifoController; 

	pDir = opendir(LOCAL_DIR);
	if(pDir==NULL){
		perror("Dir open error :");
		return;
	}

	memset(strSemNameFifo,0,MAX_FILE_NAME);
	sprintf(strSemNameFifo,"/%ld.smf",(long)gPid_server);

	getnamed(strSemNameFifo,&gSemP_fifoSem,1); 
	
	pthread_create(&th_fifoController,NULL,fifoController,NULL);

	pthread_mutex_init(&gMutex_lockSocket,NULL); // initialize mutex

	write(fdClient,&gPid_server,sizeof(pid_t));	//send pid address to client
	while(!doneflag){
		read(fdClient,&command,sizeof(Command_e));
		#ifdef DEBUG
			printf("[%ld]MiniServer read command %d from [%ld]Client\n",
				(long)gPid_server,command,(long)pidClient);
		#endif
		if(!doneflag){
			if(command==LS_CLIENT){
				lsClient(fdClient,pidClient);
			}else if(command==LIST_SERVER){
				//lock mutex and write filenames to socket
				pthread_mutex_lock(&gMutex_lockSocket); 
				listLocalFiles(pDir,fdClient);
				pthread_mutex_unlock(&gMutex_lockSocket);
			}else if(command ==DIE){
				printf("[%ld]MiniServer read die command from [%ld]Client\n",
					(long)gPid_server,(long)pidClient);
				write(fdClient,&command,sizeof(Command_e));

				doneflag=1;
				// TODO: CHANGE DONEFLAG WITH SOMETHING DIFF
			}else if(command == SEND_FILE){
				sendFile();
			}
		}
	}

	char fifoName[MAX_FILE_NAME];
	memset(fifoName,0,MAX_FILE_NAME);
	sprintf(fifoName,".%ld.ff",(long)gPid_server);
	int fd = open(fifoName,O_RDWR);
	pid_t killpid=0;
	write(fd,&killpid,sizeof(pid_t));
	close(fd);


	pthread_mutex_destroy(&gMutex_lockSocket);

	pthread_join(th_fifoController,NULL);
	closedir(pDir);
	sem_close(gSemP_fifoSem);
	pDir=NULL; // handle dangling pointers
}


void *fifoController(void *args){

	pid_t sentPid;
	char fileName[MAX_FILE_NAME];
	long filesize=0;
	char buff;
	int i=0;
	char fifoName[MAX_FILE_NAME];
	char semFifoName[MAX_FILE_NAME];

	Command_e command = SEND_FILE;
	

	memset(fifoName,0,MAX_FILE_NAME);
	sprintf(fifoName,".%ld.ff",(long)gPid_server);

	memset(semFifoName,0,MAX_FILE_NAME);
	sprintf(semFifoName,".%ld.ffsm",(long)gPid_server);

	getnamed(semFifoName,&gSemP_serverSem,1);

	mkfifo(fifoName,FIFO_PERMS);
	int fd = open(fifoName,O_RDWR);


	while(read(fd,&sentPid,sizeof(pid_t))>0 && sentPid!=0){

		memset(fileName,0,MAX_FILE_NAME);
		read(fd,fileName,MAX_FILE_NAME);
		read(fd,&filesize,sizeof(long));

		//printf("Pid : %ld, Name:%s, Size:%ld\n",(long)sentPid,fileName,filesize);
		pthread_mutex_lock(&gMutex_lockSocket);

		int cc = write(fdClient,&command,sizeof(Command_e));
		//printf("CC:%d\n",cc);
		int cp =write(fdClient,&sentPid,sizeof(pid_t));
		//printf("CP:%d\n",cp);
		int cn = write(fdClient,fileName,MAX_FILE_NAME);
		//printf("cn:%d\n",cn);
		int cs =write(fdClient,&filesize,sizeof(long));
		//printf("cs:%d\n",cs);

		for(i=0;i!=filesize;++i){
			read(fd,&buff,1);
			int a =write(fdClient,&buff,1);
			//printf("a:%d, buf:%c\n",a,buff);
		}

		pthread_mutex_unlock(&gMutex_lockSocket);
		printf("[%ld]MiniServer sent file:%s to [%ld]Client",(long)gPid_server,
							fileName,(long)sentPid);

	}

	close(fd);
	unlink(fifoName);
	sem_close(gSemP_serverSem);
	sem_unlink(semFifoName);
	return NULL;
}




// return pid of server which client connected
pid_t getClientServerPid(pid_t pidClient){

	pid_t pid=0;
	Command_e command = CHECK_CLIENT;
	sem_wait(gSemP_pipeSem);

	write(gPipe_cwpr[1],&command,sizeof(Command_e));
	write(gPipe_cwpr[1],&gPid_server,sizeof(pid_t));
	write(gPipe_cwpr[1],&pidClient,sizeof(pid_t));
	read(gPipe_crpw[0],&pid,sizeof(pid_t));
	sem_post(gSemP_pipeSem);

	return pid;
}

void sendFile(){

	char fileName[MAX_FILE_NAME];
	long filesize=0;
	pid_t pidArrival=0;
	int i=0;
	char byte;
	int sendServer=0;


	read(fdClient,&pidArrival,sizeof(pid_t));
	memset(fileName,0,MAX_FILE_NAME);
	read(fdClient,fileName,MAX_FILE_NAME);
	read(fdClient,&filesize,sizeof(long));

	printf("Send Pid:%ld, Name:%s, Size:%ld\n",(long)pidArrival,fileName,filesize);

	if(pidArrival!=1){ // send client or server
		pid_t pidArrivalServer = getClientServerPid(pidArrival);
		if(pidArrivalServer<=0){
			printf("[%ld]MiniServer will send file to server\n",(long)gPid_server);
			sendServer=1;
		}else{ // send client
			printf("[%ld]MiniServer will send file to [%ld]MiniServer-[%ld]Client pair\n",
						(long)gPid_server,(long)pidArrivalServer,(long)pidArrival);

			char fifoName[MAX_FILE_NAME];
			memset(fifoName,0,MAX_FILE_NAME);
			sprintf(fifoName,".%ld.ff",(long)pidArrivalServer);

			char semFifoName[MAX_FILE_NAME];
			memset(semFifoName,0,MAX_FILE_NAME);
			sprintf(semFifoName,".%ld.ffsm",(long)gPid_server);

			int fd = open(fifoName,O_RDWR);
			sem_t *semP;

			getnamed(semFifoName,&semP,1); // fifonun semaforunuda kilitleki
			// sadece tek kişi yazsın içine
			sem_wait(semP);
			write(fd,&pidArrival,sizeof(pid_t));
			write(fd,fileName,MAX_FILE_NAME);
			int a =write(fd,&filesize,sizeof(long));
			//printf("SizeCheck:%d - %ld\n",a,filesize);
			for(i=0;i!=filesize;++i){
				read(fdClient,&byte,1);
				int a = write(fd,&byte,1); // dosya fifoya yollandi
				//printf("-%c %d\n",byte,a);
			}

			printf("[%ld]MiniServer sent file:%s(%ld)byte to [%ld]Client\n",
					(long)gPid_server,fileName,filesize,(long)pidArrival);
			//close(fd);
			sem_post(semP);
			sem_close(semP);

		}

	}

	if(sendServer==1 || pidArrival==1){
		
		  // SEND SERVER
		// create a sem and lock file in the server
		sem_t *semServerFile;
		char semName[MAX_FILE_NAME+4];
		// prepare semaphore
		memset(semName,0,MAX_FILE_NAME+4);
		sprintf(semName,".%s.sm",fileName);
		getnamed(semName,&semServerFile,1);

		printf("[%ld]MiniServer : Waiting... File Locked!\n",(long)gPid_server);
		sem_wait(semServerFile);

		
		unlink(fileName); // delete ol files and create newfile
		int fd = open(fileName,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR| S_IRGRP | S_IROTH);

		printf("[%ld]MiniServer : File transfer to server started\n",(long)gPid_server);
		for(i=0;i!=filesize;++i){
			read(fdClient,&byte,1);
			write(fd,&byte,1);
		}

		printf("[%ld]MiniServer : File transfer to server completed\n",(long)gPid_server);
		sem_post(semServerFile);
		sem_close(semServerFile);
		close(fd);

	}
}


// pidCLient : use for debugging
void lsClient(int fdClient,pid_t pidClient){

	int clientnum=0;
	pid_t pidConnectedClient=-1;
	int i=0;
	Command_e command=LS_CLIENT;

	sem_wait(gSemP_pipeSem); // lock semafor
	pthread_mutex_lock(&gMutex_lockSocket); // lock writing socket

	write(gPipe_cwpr[1],&command,sizeof(Command_e)); // write main server
	write(gPipe_cwpr[1],&gPid_server,sizeof(pid_t)); // send mini server to mainserver
	write(fdClient,&command,sizeof(Command_e)); // say client, which command result will send
	
	read(gPipe_crpw[0],&clientnum,sizeof(int)); // read from main server
	#ifdef DEBUG
		printf("[%ld]MiniServer read %d clients pid from MainServer\n",
			(long)gPid_server,clientnum);
	#endif
	write(fdClient,&clientnum,sizeof(int)); // push client number to socket
	
	for(i=0; i!=clientnum;++i){ // read pid from pipe and write to socket
		read(gPipe_crpw[0],&pidConnectedClient,sizeof(pid_t));
		write(fdClient,&pidConnectedClient,sizeof(pid_t));
	}
	#ifdef DEBUG
		printf("[%ld]MiniServer send %d clients pid to [%ld]Client\n",
			(long)gPid_server,clientnum,(long)pidClient);
	#endif
	pthread_mutex_unlock(&gMutex_lockSocket); // unlock socket writing
	sem_post(gSemP_pipeSem); // unlock pipe semafors
}




void *listenPipe(void *args){

	Command_e command;
	pid_t pidRequest;
	// take requests from pipe
	while(!doneflag && read(gPipe_cwpr[0],&command,sizeof(Command_e))>0 && !doneflag){
		read(gPipe_cwpr[0],&pidRequest,sizeof(pid_t));
		printf("[%ld]MainServer read command from [%ld]MiniServer %d\n",
						(long)gPid_server,(long)pidRequest,command);
		if(command == LS_CLIENT){
			// first send number of clients
			write(gPipe_crpw[1],&(serverList.size),sizeof(int));
			node_t *node=serverList.head;
			while(!doneflag && node!=NULL){ //send client pids
				write(gPipe_crpw[1],&(node->data.pidClient),sizeof(pid_t));
				node=node->next;
			}
		}else if(command == CHECK_CLIENT){
			pid_t pidClient=0;
			pid_t pidServer=0;
			read(gPipe_cwpr[0],&pidClient,sizeof(pid_t));

			node_t *curr = serverList.head;
			while(curr!=NULL){
				if(curr->data.pidClient==pidClient){
					pidServer=curr->data.pidServer;
					break;
				}	
				curr=curr->next;
			}
			write(gPipe_crpw[1],&pidServer,sizeof(pid_t));
		}
		pidRequest=-1;
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
	while (((*sem = sem_open(name, 0)) == SEM_FAILED) && (errno == EINTR));

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

/*
** Reads files in directory and prints to fd
** fd can be standart output,SOCKET(i will use for this), pipe
** @param dirPath : directory path to read 
** @return : number of founded files
*/
int listLocalFiles(DIR* dir,int fd){

	struct dirent *pDirent=NULL;
	int filesize=0;
	int filenum=0;
	char fileName[MAX_FILE_NAME];
	Command_e command = LIST_SERVER;

	

	write(fd,&command,sizeof(Command_e));//first send command num to not confuse

	while((pDirent = readdir(dir))!=NULL){ // read files in directory
		if(strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0){
			if((filesize = isRegFile(pDirent->d_name))!=-1){
				memset(fileName,0,MAX_FILE_NAME);
				strcpy(fileName,pDirent->d_name);
				write(fd,fileName,MAX_FILE_NAME);
			}
		}
	}

	memset(fileName,0,MAX_FILE_NAME);
	sprintf(fileName,"/");
	write(fd,fileName,MAX_FILE_NAME);

	pDirent=NULL;
	return filenum;
}


// if regular file return file size
int isRegFile(const char * fileName){
  struct stat statbuf;

  if(stat(fileName,&statbuf) == -1){
    return -1;
  }else{
    return S_ISREG(statbuf.st_mode) ? statbuf.st_size : -1;
  }
}