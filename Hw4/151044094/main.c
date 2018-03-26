#include "stdio.h"
#include "stdlib.h"
#include <signal.h>
#include <sys/time.h>
#include "listdir.h"
static volatile sig_atomic_t exit_flag=0;

static void notExit(int signo){
	exit_flag=1;
}

int main(int argc,char * argv[])
{
	int numberOfNeedle;
	char * needle;
	char * path;
	FILE *logPtr;
	struct timeval start,end;
	float timeCount;
	gettimeofday(&start,0);
	logPtr=fopen("log.txt","a+");

	struct sigaction act;
	
	act.sa_handler=notExit;
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
	
	// Usage
	if(argc!=3)
	{
		printf("Proper usage of this program is\n./grepTh <string> <directory>\n");
		return -1;
	}

	needle=argv[1];
	// Checking if the directory exists
	path=argv[2];
	Result r1=listdir(needle,path,logPtr);
	if(r1.numberOfStrings==-1)
		return -1;
	else
	{
		printf("Successful! All the log can be found in log.txt\n");
		fprintf(logPtr,"-------------------------------------------------\n");
		fprintf(logPtr,"%d %s were found in total.\n",r1.numberOfStrings,needle);
		printf("Total number of %s found:	%d\n",needle,r1.numberOfStrings);
		printf("Number of directories searched:	%d\n",r1.numberOfDirectories+1);
		printf("Number of files searched:	%d\n",r1.numberOfFiles);
		printf("Number of lines searched:	%d\n",r1.numberOfLines);
		printf("Number of cascade threads created:	%d\n",r1.cascadeThreads);
		printf("Number of search threads created:	%d\n",r1.numberOfFiles);
		printf("Max # of threads running concurrently:	%d\n",r1.maxOfThreads);
		gettimeofday(&end,0);
		timeCount=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000;
		printf("Total run time, in milliseconds:	%.2f\n",timeCount);
		if(exit_flag==1)
			printf("Exit condition:(CTRL+C or CTRL+Z pressed)\n");
		else
			printf("Exit condition:(normal)\n");
	}
	fclose(logPtr);
	return 0;
}