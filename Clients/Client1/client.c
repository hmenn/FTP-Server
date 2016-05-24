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
#include "hmlinkedlist.h"

#define DEBUG 0

typedef enum{LIST_SERVER,LS_CLIENT,CHECK_CLIENT,SEND_CLIENT,
	SEND_SERVER}Command_e;

#define MAX_LINE_LEN 80

static sig_atomic_t doneflag=0;
int fdSocket;

void sighandler(int signo){
	printf("Signal\n");
	doneflag=1;
	close(0); // close for fgets
}

void * listenSocket(void *args){

	

	
}

// returns socket fd
int connectServer(const char *ipnum,int portnum);
void showHelpManual();
int isRegFile(const char * fileName);

hmlist_t* readDirectory(const char *dirpath);

int main(int argc, char *argv[]){

	if(argc!=3){
		fprintf(stderr,"USAGE: %s ipnum portnum\n",argv[0]);
		return 0;
	}

	signal(SIGINT,sighandler);
	char line[MAX_LINE_LEN];
	int portnum = atoi(argv[2]);
	int fdSocket = connectServer(argv[1],portnum);
	pid_t pid;
	hmlist_t *filelist;
	if(fdSocket == -1){
		perror("->Main : socked fildes");
		return 1;
	}

	pid=getpid();
	write(fdSocket,&pid,sizeof(pid_t));
	printf("Connection completed.\n");
	printf("Let's make some magic...\n");

	while(!doneflag){
		memset(line,0,MAX_LINE_LEN);
		printf("Enter command ");
		if(fgets(line,MAX_LINE_LEN,stdin)!=NULL && !doneflag){

			line[strlen(line)-1]='\0'; // delete \n character
			if(strcmp(line,"help")==0){ // help command
				showHelpManual();
			}else if(strcmp(line,"listLocal")==0){
				filelist = readDirectory("./");
				printList(filelist);
				deleteList(filelist);
				free(filelist);
			}else if(strcmp(line,"lsClient")==0){
				int command;
				pid_t connedtedPid;
				int i=1;
				command = LS_CLIENT;
				write(fdSocket,&command,sizeof(int));
				printf("Connedted Client Pids...\n");
				while(read(fdSocket,&connedtedPid,sizeof(pid_t)) >0 &&
					connedtedPid != -1){
					printf("%d. %ld\n",i,(long)connedtedPid);
					++i;
				}
			}else{
				printf("#Entered invalid command\n");
				printf("#Please check help page\n");
			}
		}
	}

	close(fdSocket);

	printf("\n\nCLIENT CLOSED SUCCUSSFULLY\n\n");
	return 0;
}

void showHelpManual(){
	printf("\n#################################\n");
	printf("Arguments...\n");
	printf("1. help : show help manual\n");
	printf("2. listLocal : to list the local files in the directory client program started\n");
	printf("3. listServer : to list the files in the current scope of the server-client\n");
	printf("4. lsClient pid : lists the clients currently connected to the server with their respective clientids\n");
	printf("5. sendFile <filename> <clientid>  : send the file <filename> (if file exists) from \
		local directory to the client with client id clientid. If no client id is given the  \
		file is send to the servers local directory.\n");
	printf("\n#################################\n");
	printf("\n");
}



void sigAlarmHandler(int signum){
	doneflag=1;
	close(fdSocket);
}


int connectServer(const char *ipnum,int portnum){

	struct sockaddr_in serverAddr;
	signal(SIGALRM,sigAlarmHandler);
	
	


	if((fdSocket=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("->ConnecServer : socket error");
		return -1;
	}


	memset(&serverAddr,0,sizeof(struct sockaddr_in));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=inet_addr(ipnum);
	serverAddr.sin_port=htons(portnum);

	alarm(2); // eger 2 sn icinde sockete 
	if(connect(fdSocket,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_in))<0){
		perror("->ConnectServer : connect error");
		return -1;
	}
	alarm(0);

	return fdSocket;
}


hmlist_t* readDirectory(const char *dirpath){

	DIR *pDir;
	struct dirent *pDirent=NULL;
	hmlist_t *files=NULL;

	pDir = opendir(dirpath);
	if(pDir==NULL){
		perror("Dir open error : ");
		return NULL;
	}

	files = (hmlist_t *)malloc(sizeof(hmlist_t));
	memset(files,0,sizeof(hmlist_t));
	char strPath[MAX_FILE_NAME];
	while((pDirent = readdir(pDir))!=NULL){
		memset(strPath,0,MAX_FILE_NAME);
		sprintf(strPath,"%s/%s",dirpath,pDirent->d_name);
		if(strcmp(strPath,".")!=0 && strcmp(strPath,"..")!=0){
			int filesize;
			if((filesize = isRegFile(strPath))!=-1){
				hmFile_t file;
				memset(&file,0,sizeof(hmFile_t));
				strcpy(file.filename,strPath);
				file.filesize = filesize;
				addLast(files,&file);
			}
		}
	}
	closedir(pDir);
	return files;

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
