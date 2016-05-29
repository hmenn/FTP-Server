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
#include "cl.h"


static sig_atomic_t doneflag=0;
static sig_atomic_t sigPrint=0;
int gI_socketFd;
pid_t gPid_client;
int gPipe_listenSocket[2]; // socketten gelenler buradan aktarilacak


int main(int argc,char *argv[]){

	if(argc!=3){
		fprintf(stderr, "USAGE : %s ipnum portnum\n",argv[0]);
		return 0;
	}

	int i=1; // counter
	signal(SIGINT,sigHandler);
	char line[MAX_LINE_LEN];
	int portnum = atoi(argv[2]);
	pid_t pidServer=-1;
	gI_socketFd = connectServer(argv[1],portnum);
	DIR *pDir=NULL;

	if(gI_socketFd == -1){
		perror("Invalid port/socket");
		return 1;
	}

	gPid_client = getpid();
	// connect with pid and take server pid addres
	write(gI_socketFd,&gPid_client,sizeof(pid_t));
	read(gI_socketFd,&pidServer,sizeof(pid_t));

	printf("##[%ld]Client connected to [%ld]server\n",(long)gPid_client,(long)pidServer);

	pipe(gPipe_listenSocket);
	pthread_t th_sockListener;
	pthread_create(&th_sockListener,NULL,socketListener,NULL);

	pDir = opendir(LOCAL_DIR);
	if(pDir==NULL){
		perror("Dir open error :");
		close(gI_socketFd);
		return -1;
	}

	
	Command_e command; // command value
	while(!doneflag){
		memset(line,0,MAX_LINE_LEN);
		printf("Enter your command : ");
		if(fgets(line,MAX_LINE_LEN,stdin)!=NULL && !doneflag){
			line[strlen(line)-1]='\0';
			if(strcmp(line,"help")==0){
				showHelpManual();
			}else if(strcmp(line,"listLocal")==0){ // list local files
				listFilesInDir(pDir);
			}else if(strcmp(line,"lsClient")==0){ // list client pids
				lsClient();
			}else if(strcmp(line,"listServer")==0){ // list server files
				listServer(pidServer);
			}else if(strstr(line,"sendFile")!=NULL){ // send file
				sendFileWrapper(pDir,line);
			}else if(strcmp(line,"exit")==0){
				doneflag=1;
			}else{
				printf("\n##Entered invalid command\n");
				printf("##Please check 'help' page\n\n");
			}
		}else{
			doneflag=1;
		}
	}

	if(doneflag){ // send die command to server and thread
		command = DIE;
		write(gI_socketFd,&command,sizeof(Command_e));
	}

	// when server die, will send thread die command end thread die
	pthread_join(th_sockListener,NULL);
	closedir(pDir);
	close(gI_socketFd);
	pDir=NULL; // handle dangling pointers
	if(sigPrint)
		printf("\nCTRL+C HANDLED\n");
	printf("\nCLIENT CLOSED SUCCUSSFULLY\n\n");
	return 0;
}

// it's a wrapper function to control send file operation and reduce main 
// pollution
void sendFileWrapper(DIR* dir,char *line){
	char *strName=NULL;
	char *strPid=NULL;
	pid_t pidClient;
	strtok(line," ");
	strName = strtok(NULL," ");
	strPid = strtok(NULL," ");
	if(strName == NULL){
		fprintf(stderr,"\nInvalid sendFile parameter. Check help manual\n\n");
	}else{
		if(strPid == NULL){
			pidClient=0; // send server
		}else pidClient = atoi(strPid);
		#ifdef DEBUG
			printf("send file:%s to pid:%ld\n",strName,(long)pidClient);
		#endif
		sendFile(dir,strName,pidClient);
	}
	strName=NULL;
	strPid=NULL;
}

// if file in directory, client will send directory to pid address
// if pid 0, file will send server
int sendFile(DIR* dir, const char *fileName,pid_t pid){

	Command_e command=SEND_FILE;
	long filesize=0;
	if((filesize=isFileInDir(dir,fileName))==-1){
		fprintf(stderr, "File:%s not found in local dir\n",fileName);
		return -1;
	}

	write(gI_socketFd,&command,sizeof(Command_e));
	write(gI_socketFd,&pid,sizeof(pid_t));
	write(gI_socketFd,fileName,MAX_FILE_NAME);
	write(gI_socketFd,&filesize,sizeof(long));

	int fd = open(fileName,0600); // read file byte-byte

	char ch;
	while(read(fd,&ch,1)>0){
		write(gI_socketFd,&ch,1); //send file
	}
	
	close(fd);
	printf("[%ld]Client sent file\n",(long)gPid_client);
}

// Checks file already in dir
// @return : if file there , return filesize otherise return -1
// @param  dir pointer
// @param  filename : filename to check 
long isFileInDir(DIR *dir,const char *fileName){
	struct dirent *pDirent;
	int found=0;
	long filesize=-1;
	struct stat st;

	rewinddir(dir);
	while((pDirent = readdir(dir))!=NULL){
		if(strcmp(fileName,pDirent->d_name)==0){
			stat(fileName,&st);
			filesize=st.st_size;
			found=1;
			break;
		}
	}
	pDirent=NULL;
	return found==1 ? filesize : -1;
}


// sends request to mini server and takes list of files in server
// @param : server pid to print details of step
void listServer(pid_t pidServer){
	char fileName[MAX_FILE_NAME];
	Command_e command = LIST_SERVER;
	int i=0;

	printf("\n############################\nCommand: LIST FILES IN SERVER\n");
	write(gI_socketFd,&command,sizeof(Command_e)); // send request
	printf("##[%ld]Send request to [%ld]server\n\n",(long)gPid_client,(long)pidServer);

	memset(fileName,0,MAX_FILE_NAME);
	
	while(read(gPipe_listenSocket[0],fileName,MAX_FILE_NAME)>0){
		if(strcmp(fileName,INVALID_FILE_NAME)==0)
			break;
		++i;
		printf("%d. %s\n",i,fileName);
	}
	printf("\nThere are %d files in server\n",i);
	printf("\n###########################\n");
}

// sends request to server and takes online client pid's
void lsClient(){
	pid_t pidOnlineClient=-1;
	Command_e command = LS_CLIENT;
	int size=0;
	int i=0;
	// get online clients pid request
	write(gI_socketFd,&command,sizeof(Command_e));
	// read results from thread/pipe connections 
	printf("\n############################\nCommand : LIST CLIENT PID'S\n");
	printf("Waiting for list of online clients\n");
	
	read(gPipe_listenSocket[0],&size,sizeof(int));
	printf("\nThere are %d online client\n",size);
	
	for(i=1;i<=size;++i){
		read(gPipe_listenSocket[0],&pidOnlineClient,sizeof(pid_t));
		printf("%d. %ld\n",i,(long)pidOnlineClient);
	}
	printf("\n###########################\n");
}

// signal handler
void sigHandler(int signum){
	doneflag=1;
	sigPrint=1;
	close(0);
}


// show manual page
void showHelpManual(){
	printf("\n#################################\n");
	printf("......HELP MANUAL......\n");
	printf("\nCommands: \n");
	printf("\n-> exit : close program normally\n");
	printf("\n-> help : show help manual\n");
	printf("\n-> listLocal :list the local files in the directory client program started\n");
	printf("\n-> listServer : list the files in the current scope of the server-client\n");
	printf("\n-> lsClient pid : lists the clients currently connected to the server with their respective clientids\n");
	printf("\n-> sendFile filename clientid  : send the file <filename>");
	printf("(if file exists) from local directory to the client with client");
	printf("id clientid. If no client id is given the file is send to the\nservers local directory.\n");
	printf("\n#################################\n");
	printf("\n");
}

// sig alarm handler
void sigAlarmHandler(int signum){
	doneflag=1;
	close(gI_socketFd);
}


// this function will use by liestener thread
// thread will listen socked and accept request or file transfers and direct
// coming data to real owner
void *socketListener(void *args){
	Command_e command;
	int size=0;
	int i=0;
	pid_t pidOtherClient;
	char fileName[MAX_FILE_NAME];

	while(read(gI_socketFd,&command,sizeof(Command_e))>0){
		if(command == LS_CLIENT){ 
			size=0;
			read(gI_socketFd,&size,sizeof(int)); // read number of pids
			write(gPipe_listenSocket[1],&size,sizeof(int)); // send nums to pipe
			
			for(i=0;i<size;++i){
				// pid leri oku ve main threade yolla
				read(gI_socketFd,&pidOtherClient,sizeof(pid_t));
				write(gPipe_listenSocket[1],&pidOtherClient,sizeof(pid_t));
			}
		}else if(command==LIST_SERVER){
			memset(fileName,0,MAX_FILE_NAME);
			while(read(gI_socketFd,fileName,MAX_FILE_NAME)>0){
				write(gPipe_listenSocket[1],fileName,MAX_FILE_NAME);		
				if(strcmp(fileName,INVALID_FILE_NAME)==0)
						break;
			}
		}else if(command==DIE){
			break;
		}else if(command==SEND_FILE){

			pid_t whoSent=0;
			long filesize=0;
			char fileName[MAX_FILE_NAME];
			char buff;

			memset(fileName,0,MAX_FILE_NAME);
			read(gI_socketFd,&whoSent,sizeof(pid_t));
			read(gI_socketFd,fileName,MAX_FILE_NAME);
			read(gI_socketFd,&filesize,sizeof(long));
			printf("WhoSent:%ld, Name:%s, Size:%ld\n",(long)whoSent,fileName,filesize);

			strcat(fileName,"-new");
			unlink(fileName); 
			int fd =open(fileName,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR| S_IRGRP | S_IROTH);

			// delete ol files and create newfile

			for(i=0;i!=filesize;++i){
				read(gI_socketFd,&buff,1);
				write(fd,&buff,1);
			}

			printf("[%ld]MiniServer : File transfer to server completed\n",(long)gPid_client);

			close(fd);
		}
	}
	return NULL;
}


// Send request to main server and then connects mini server
// if server dont answer in 2sec, will return error
// @param ipnum : ip number of server
// @param portnum : number of main server 
int connectServer(const char *ipnum,int portnum){
	struct sockaddr_in serverAddr;
	socklen_t serverArrLen;
	signal(SIGALRM,sigAlarmHandler);
	
	if((gI_socketFd=socket(AF_INET,SOCK_STREAM,0))<0){
		return -1;
	}

	serverArrLen = sizeof(struct sockaddr_in);
	memset(&serverAddr,0,serverArrLen);
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=inet_addr(ipnum);
	serverAddr.sin_port=htons(portnum);

	alarm(2); // wait 2second to connect
	if(connect(gI_socketFd,(struct sockaddr*)&serverAddr,serverArrLen)<0){
		return -1;
	}
	alarm(0);

	return gI_socketFd;
}

/*
** Reads files in directory and prints standart output
** @param dirPath : directory path to read 
** @return : number of founded files
*/
int listFilesInDir(DIR *dir){

	struct dirent *pDirent=NULL;
	struct stat st;
	int filenum=0;

	printf("#########################\nCommand : List Local Files\n");
	printf("Reading files in local dir ...\n\n");
	while((pDirent = readdir(dir))!=NULL){ // read files in directory
		if(strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0){
			stat(pDirent->d_name,&st);
			++filenum;
			printf("%d. %s\t %ld(byte)\n",filenum,pDirent->d_name,(long)st.st_size);
		}
	}
	printf("\nFound %d files\n#########################\n",filenum);

	pDirent=NULL; // handle dangling ptr
	return filenum;
}

