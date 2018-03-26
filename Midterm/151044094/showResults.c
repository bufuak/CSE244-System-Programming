#include "matrix.h"

void showResults();

static volatile sig_atomic_t exit_flag=0;
static volatile sig_atomic_t program_flag=0;

static void setExitFlag(int signo){
	exit_flag=1;
}
static void notExit(int signo){
	exit_flag=0;
}
static void setProgramFlag(int signo){
	program_flag=1;
}


int main(int argc,char * argv[])
{
	showResults();
	return 0;
}

void showResults()
{
	FILE *logPtr;
	char* client_to_showResults="showResultsFifo";
	int fifo1;
	int fifo2;
	int clientPid;
	int thisPid;
	char result_to_client[255];
	int clientPids[20];
	char logName[255];
	struct sigaction act;

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
	act.sa_handler=setProgramFlag;
	act.sa_flags=3;
	if(sigaction(SIGUSR2,&act,NULL)==-1)
	{
		perror("Failed to set SIGUSR2\n");
		return;
	}

	sprintf(logName,"log/log_showResults%d",getpid());
	
	struct stat st = {0};

	if (stat("./log", &st) == -1) {
	    mkdir("./log", 0700);
	}

	logPtr=fopen(logName,"a+");

	if((fifo1 = open(client_to_showResults,O_RDONLY|O_NONBLOCK)) < 0) 
    {
    	fprintf(stderr,"seeWhat is not running\n");
    	exit(1);
    }

    thisPid=getpid();
    int clientCount=0;
    int count=0;
    while(!exit_flag)
    {
    	ClientResult cr;
    	if(program_flag==1)
    	{
    		fprintf(logPtr,"seeWhat or timerServer is closed. I'll kill myself\n");
    		fclose(logPtr);
    		close(fifo1);
    		return;
    	}
		int somenumber=read(fifo1, &cr, sizeof(cr));
		if(somenumber>0)
		{
			clientPid=cr.pid;
			int i;
			int condition=0;
			for(i=0;i<clientCount;i++)
			{
				if(clientPids[i]==clientPid)
				{
					condition=1;
					break;
				}
			}
			if(condition!=1)
				clientPids[clientCount++]=clientPid;
			printf("Client(%d):	",clientPid);
			printf("Result1:%f 	Result2:%f\n",cr.r1.result,cr.r2.result);
			fprintf(logPtr,"m%d_%d\n",count++,clientPid);
			fprintf(logPtr,"Result1:%.3f,Time:%ld\n",cr.r1.result,cr.r1.time);
			fprintf(logPtr,"Result1:%.3f,Time:%ld\n",cr.r2.result,cr.r2.time);
		}
    }
    int i;
    for(i=0;i<clientCount;i++)
    	kill(clientPids[i],SIGUSR2);
	close(fifo1);
	fprintf(logPtr,"CTRL+C pressed\n");
	fclose(logPtr);
	return;
}
