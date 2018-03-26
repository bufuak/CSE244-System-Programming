#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "matrix.h"
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>  
#include <errno.h>
#include <time.h>
#include <math.h>
#include <linux/limits.h>

#define FIFO_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define FIFO_MODES (O_RDONLY|O_WRONLY)

static volatile sig_atomic_t signal_flag=0;
static volatile sig_atomic_t exit_flag=0;

static void setSignalFlag(int signo){
	signal_flag=1;
}

static void setExitFlag(int signo){
	exit_flag=1;
}

struct client
{
    int sockfd;
    int pid;
    long int tid;
    int m;
    int p;
    int portno;
    int numberOfClient;
    pthread_t tids[50];
    double times[50];
    struct sockaddr_in serv_addr;
};
typedef struct client Client;

sem_t   sem;

void error(char *msg)
{
    perror(msg);
    exit(0);
}


void *function(void *arg)
{
	clock_t start=clock();		
    Client *clientptr=arg;
    int sockfd, portno, n,i;
    FILE *dumb;
    int server_to_client;
    long int thisTid;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int clientNo;
    for(i=0;i<clientptr->numberOfClient;i++)
    {
    	if(clientptr->tids[i]==pthread_self())
    		clientNo=i;
    }
    thisTid=(long int)pthread_self();
    char logName[255];
    sprintf(logName,"log/log_client%d-%ld",getpid(),thisTid);
    FILE* logPtr;
    struct stat st = {0};
    if (stat("./log", &st) == -1) {
	    mkdir("./log", 0700);
	}
	logPtr=fopen(logName,"a+");
  
  	char fifoName[255];
  	sprintf(fifoName,"client%d-%ld",getpid(),thisTid);
    if(mkfifo(fifoName,FIFO_PERM)<0)
	{
		if(errno==EEXIST)
		{
			fprintf(stderr,"(%ld) Bu isimde bir fifo halihazırda var  %s: %s\n",(long)getpid(),fifoName,strerror(errno));
			exit(-1);
		}
	}
	dumb=fopen(fifoName,"a+"); // This file pointer is never used.

	server_to_client=open(fifoName,O_RDONLY);
	
    portno = clientptr->portno;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    
    sem_wait(&sem);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        close(server_to_client);
	    unlink(fifoName);
	    fclose(dumb);
	    error("Server should be closed without work is done!\n || Wrong portno\n");
	    return;
    }
    clientptr->tid=thisTid;
    n = write(sockfd,&clientptr->pid,sizeof(int));
    if (n < 0) 
         error("ERROR writing to socket");

    n = write(sockfd,&clientptr->tid,sizeof(long int));
    if (n < 0) 
         error("ERROR writing to socket");

    n = write(sockfd,&clientptr->m,sizeof(int));
    if (n < 0) 
         error("ERROR writing to socket");

    n = write(sockfd,&clientptr->p,sizeof(int));
    if (n < 0) 
         error("ERROR writing to socket"); 
    sem_post(&sem);

    Matrix mat;
    int bytes=0;
    while(1)
    {
    	bytes = read(server_to_client,&mat,sizeof(Matrix));
    	if(bytes>0)
    		break;
    }

    int a,b;
	fprintf(logPtr,"Matrix is:\n[");
    for(a=0;a<mat.m;a++)
	{
		for(b=0;b<mat.p;b++)
		{
			fprintf(logPtr,"%.2f",mat.A[a][b]);
			if(b==mat.p-1)
			{
				if(a==mat.m-1)
				{
					fprintf(logPtr,"]\n");
				}
				else
				{
					fprintf(logPtr,";\n");	
				}
			}
			else
			{
				fprintf(logPtr,",");
			}
			//printf("%.2f 	",mat.matrix[a][b]);

		}
		//printf("\n");
	}
	fprintf(logPtr,"\n");
	fprintf(logPtr,"b is:\n[");
	{
		for(a=0;a<mat.m;a++)
		{
			fprintf(logPtr,"%.2f",mat.b[a]);
			if(a==mat.m-1)
			{
				fprintf(logPtr,"]\n");
			}
			else
			{
				fprintf(logPtr,",");
			}
		}
	}
	fprintf(logPtr,"\n");


	fprintf(logPtr,"solution is:\n[");
	{
		for(a=0;a<mat.p;a++)
		{
			fprintf(logPtr,"%.2f",mat.x[a]);
			if(a==mat.p-1)
			{
				fprintf(logPtr,"]\n");
			}
			else
			{
				fprintf(logPtr,",");
			}
		}
	}
	fprintf(logPtr,"\n\nError is: %f",mat.error);

	fclose(logPtr);
	
    close(server_to_client);
    unlink(fifoName);
    fclose(dumb);
    clock_t end=clock();
	clientptr->times[clientNo]=(double)(end-start)/CLOCKS_PER_SEC;
    return NULL;
}


int main(int argc, char *argv[])
{
    sem_init(&sem,0,1);
	
    if (argc < 5) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    FILE *logPtr;
    char logName[255];
    struct stat st = {0};
    if (stat("./log", &st) == -1) {
	    mkdir("./log", 0700);
	}
   	sprintf(logName,"log/client%d",getpid());
   	logPtr=fopen(logName,"a+");
    Client client;
    client.pid=getpid();
    client.portno=atoi(argv[1]);
    client.m=atoi(argv[2]);
    client.p=atoi(argv[3]);
    if(client.p>client.m)
    {
    	fprintf(stderr,"#Column can not be more than #rows\n");
    	fclose(logPtr);
    	return;
    }
    client.numberOfClient=atoi(argv[4]);
    Client *clientptr;
    clientptr=&client;
    int i;
    double totaltime=0;
    for(i=0;i<client.numberOfClient;i++)
    {	
	    int err = pthread_create(&(client.tids[i]), NULL, &function,clientptr);
	    if (err != 0)
	        printf("\ncan't create thread :[%s]", strerror(err));
	}
    int j;
    for(j=0;j<i;j++)
    {
        pthread_join(client.tids[j],NULL); 
        totaltime+=client.times[j];
    }
    double average=totaltime/client.numberOfClient;
    totaltime=0;
    for(j=0;j<i;j++)
    {
    	totaltime+=pow(client.times[j]-average,2);
    }
    double standart=totaltime/(i-1);
    fprintf(logPtr,"Average is: %lf\nStandart deviation is: %lf\nFor m:%d p:%d clientNumber:%d\n",average,standart,client.m,client.p,client.numberOfClient);
    fclose(logPtr);
    return 0;
}
