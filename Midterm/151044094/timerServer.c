#include "matrix.h"

void timerServer(int mSec,int matrixN,char * mainProccess);

static volatile sig_atomic_t exit_flag=0;
static volatile sig_atomic_t signal_flag=0;
static volatile sig_atomic_t program_flag=0;

static void setExitFlag(int signo){
	exit_flag=1;
}
static void notExit(int signo){
	exit_flag=0;
}
static void setSignalFlag(int signo){
	signal_flag=1;
}
static void setProgramFlag(int signo){
	program_flag=1;
}

int main(int argc,char * argv[])
{
	int ticksInMsec;
	int n;
	char * mainProccess;
	// Usage:
	if(argc!=4)
	{
		printf("Proper usage of this program is:\n");
		printf("./timerServer <ticksInMsec(int)>"); 
		printf(" <n(integer)> <mainProccess(string)>\n"); 
		return -1; // Error -1 
	}

	ticksInMsec= atoi(argv[1]); // Ticks in msec
	n= atoi(argv[2]); // Matrix' order
	mainProccess=argv[3]; // Main fifo
	if(ticksInMsec<=0)
	{
		printf("ticksInMsec value can not be 0 or negative\n");
		return 0;
	}
	if(n<=0)
	{
		printf("Matrix's order can not be 0 or negative.\n");
		return 0;
	}
	timerServer(ticksInMsec,n,mainProccess);
	return 0;
}

void timerServer(int mSec,int matrixN,char * mainProccess)
{
	FILE *a,*b,*c,*logPtr;
	int server_to_client;
	int client_to_server;
	int server_to_client_matrix;
	int readBytes;
	int thisPid;
	int childPids[20];
	int childPid;
	int clientPids[20];
	int clientPid;
	char logName[255];
	char clientFifo[255];
	char secondPipe[255];
	struct sigaction act;
	char currentTime[64];
	
	act.sa_handler=setExitFlag;
	act.sa_flags=0;
	if((sigemptyset(&act.sa_mask)==-1)||
		(sigaction(SIGINT,&act,NULL)==-1))
	{
		perror("Failed to set SIGINT handler\n");
		return;
	}
	act.sa_handler=notExit;
	act.sa_flags=1;
	if(sigaction(SIGQUIT,&act,NULL)==-1||
		sigaction(SIGTSTP,&act,NULL)==-1)
	{
		perror("Failed to set SIGQUIT and SIGTSTP\n");
		return;
	}
	act.sa_handler=setSignalFlag;
	act.sa_flags=3;
	if(sigaction(SIGUSR1,&act,NULL)==-1)
	{
		perror("Failed to set SIGUSR1\n");
		return;
	}
	act.sa_handler=setProgramFlag;
	act.sa_flags=4;
	if(sigaction(SIGUSR2,&act,NULL)==-1)
	{
		perror("Failed to set SIGUSR2\n");
		return;
	}

	sprintf(logName,"log/log_timerServer%d",getpid());
	struct stat st = {0};

	if (stat("./log", &st) == -1) {
	    mkdir("./log", 0700);
	}
	logPtr=fopen(logName,"a+");

	if(mkfifo(mainProccess,FIFO_PERM)<0)
	{
		if(errno==EEXIST)
		{
			fprintf(stderr,"(%ld) Bu isimde bir fifo halihazırda var  %s: %s\n",(long)getpid(),mainProccess,strerror(errno));
			exit(-1);
		}
	}
	a=fopen(mainProccess,"a+"); // This file pointer is never used.

	thisPid=getpid();
	server_to_client=open(mainProccess,O_WRONLY);
	//printf("Server's pid is: %d\n",thisPid);
	write(server_to_client, &thisPid, sizeof(thisPid));

	
	sprintf(secondPipe,"server%d", thisPid);
	if(mkfifo(secondPipe,FIFO_PERM) < 0) {
	    if(errno!=EEXIST)
		{
			fprintf(stderr,"(%ld):failed to create named pipe %s: %s\n",(long)getpid(),secondPipe,strerror(errno));
			exit(-1);
		}
  	}
  	b=fopen(secondPipe,"a+"); // This file pointer is never used.

	client_to_server=open(secondPipe,O_RDONLY);
	//printf("This pid:%d, Server's pid:%d\n",thisPid,getpid());
	int clientCount=0;
	while(!exit_flag)
	{
		//printf("Signal flag is:%d for %d\n",signal_flag,getpid());
		if(signal_flag==1)
		{
			if(thisPid==getpid())
			{
				close(server_to_client);
				server_to_client=open(mainProccess,O_WRONLY);
				write(server_to_client, &thisPid, sizeof(thisPid));
				//printf("seeWhat is alive.Requesting matrix\n");
				readBytes = read(client_to_server,&clientPid,sizeof(clientPid));
				if(readBytes>0) // New client;
				{
					//printf("New client:%d came.\n",clientPid);
					clientPids[clientCount]=clientPid;
					childPid=fork();
					childPids[clientCount++]=childPid;
					if(thisPid==getpid())
					{
						//printf("I am your father Luke!\n");
					}	
					else
					{
						sprintf(clientFifo,"client%d",clientPid);
						//printf("NOOOOOO!\n");
					}
				}
				signal_flag=0;
			}
		}
		
		if(thisPid!=getpid())
		{
			int a,b,c,d;
			Matrix matrix,m1,m2,m3,m4;
			int size=matrixN;
			matrix.size=size*2;
			m1.size=size;m2.size=size;m3.size=size;m4.size=size;
			Matrix *matrixptr=&matrix;
			Matrix *m1ptr=&m1;
			Matrix *m2ptr=&m2;
			Matrix *m3ptr=&m3;
			Matrix *m4ptr=&m4;
			randomInvertibleMatrix(m1ptr);
			/*for(a=0;a<m1.size;a++)
			{
				for(b=0;b<m1.size;b++)
					printf("%.2f 	",m1.matrix[a][b]);
				printf("\n");
			}*/
			//printf("\n");
			randomInvertibleMatrix(m2ptr);
			/*for(a=0;a<m2.size;a++)
			{
				for(b=0;b<m2.size;b++)
					printf("%.2f 	",m2.matrix[a][b]);
				printf("\n");
			}*/
			//printf("\n");
			randomInvertibleMatrix(m3ptr);
			/*for(a=0;a<m3.size;a++)
			{
				for(b=0;b<m3.size;b++)
					printf("%.2f 	",m3.matrix[a][b]);
				printf("\n");
			}*/
			//printf("\n");
			randomInvertibleMatrix(m4ptr);
			/*for(a=0;a<m4.size;a++)
			{
				for(b=0;b<m4.size;b++)
					printf("%.2f 	",m4.matrix[a][b]);
				printf("\n");
			}*/
			//printf("\n");
			
			for(a=0;a<matrix.size/2;a++)
			{
				for(b=0;b<matrix.size/2;b++)
				{
					matrix.matrix[a][b]=m1.matrix[a][b];	
				}
			}
			
			for(a=0;a<matrix.size/2;a++)
			{
				c=0;
				for(b=matrix.size/2;b<matrix.size;b++)
				{
					matrix.matrix[a][b]=m2.matrix[a][c++];	
				}
			}
			
			c=0;
			for(a=matrix.size/2;a<matrix.size;a++)
			{
				for(b=0;b<matrix.size/2;b++)
				{
					matrix.matrix[a][b]=m3.matrix[c][b];	
				}
				c++;
			}
			c=0;
			for(a=matrix.size/2;a<matrix.size;a++)
			{
				d=0;
				for(b=matrix.size/2;b<matrix.size;b++)
				{
					matrix.matrix[a][b]=m4.matrix[c][d++];	
				}
				c++;
			}

			/*
			printf("Matrix:\n\n");
			for(a=0;a<matrix.size;a++)
			{
				for(b=0;b<matrix.size;b++)
					printf("%.2f 	",matrix.matrix[a][b]);
				printf("\n");
			}
			*/
			
			matrix.determinant=determinant(matrix.matrix,matrix.size);
			time_t t=time(NULL);
			struct tm *tm=localtime(&t);
			strftime(currentTime,sizeof(currentTime),"%c",tm);
			fprintf(logPtr,"time of matrix generated: in %s, pid of client: %d, determinant of the matrix: %f\n",currentTime,clientPid,matrix.determinant);
		    if((server_to_client_matrix = open(clientFifo,O_WRONLY)) < 0) 
			{
				close(server_to_client);
				close(client_to_server);
				perror(clientFifo);
				exit(1);
			}
			write(client_to_server,&matrix,sizeof(matrix));
			close(client_to_server);
		}
		if(program_flag==1)
		{
			if(thisPid==getpid())
			{
				fprintf(logPtr,"seeWhat or showResuls is dead. I'll kill myself\n");
				int i;
				//printf("And killing my children :(\n");
				for(i=0;i<clientCount;i++)
				{
					//printf("Farewell: Child: %d\n",childPids[i]);
					kill(childPids[i],SIGINT);
				}
				
				close(server_to_client);
				close(client_to_server);
				fclose(logPtr);
				fclose(a);
				fclose(b); // dumb file pointers closed.
				unlink(secondPipe);
				unlink(mainProccess);
				//printf("And killing other seeWhats\n");
				
				for(i=0;i<clientCount;i++)
				{
					//printf("Killing: Client %d\n",clientPids[i]);
					kill(clientPids[i],SIGUSR2);
				}
				system("killall -SIGUSR2 showResults"); // Killing showResults
				exit(-1);	
			}
		}
		sleep((double)mSec/(double)1000);
	}

	if(thisPid==getpid())
	{
		int i;
		fprintf(logPtr,"CTRL+C pressed. timerServer ending!\n");
		//printf("And killing other seeWhats\n");
		
		for(i=0;i<clientCount;i++)
		{
			//printf("Killing: Client %d\n",clientPids[i]);
			kill(clientPids[i],SIGUSR2);
		}
		system("killall -SIGUSR2 showResults");
		close(server_to_client);
		close(client_to_server);
		fclose(logPtr);
		fclose(a);
		fclose(b); //dumb file pointers closed
		unlink(secondPipe);
		unlink(mainProccess);
		return;
	}
	else
	{
		close(server_to_client);
		fclose(logPtr);
		fclose(a);
		fclose(b); // dumb file pointers closed.
		close(client_to_server);
		exit(-1);
	}
}



