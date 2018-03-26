#include "dirent.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "listdir.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>	
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define MAX_PATH_SIZE 255
#define FIFO_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define FIFO_MODES (O_RDONLY|O_WRONLY)
#define PERMS (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *function(void *arg)
{
	unsigned long i = 0;
    pthread_t id = pthread_self();
    ThreadResult *r=arg;
    /*
    int shmid;
    key_t key;
    ListResult *result;
.
     
    key = getpid()*2;
    
    if ((shmid = shmget(key, sizeof(ListResult), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
    
    if ((result = shmat(shmid, NULL, 0)) == (ListResult *) -1) {
        perror("shmat");
        exit(1);
    }
	*/
    for(i=0;i<r->numberOfFiles;i++)
    {
    	if(pthread_equal(id,r->tid[i]))
    	{
        	ListResult lResult=list(r->paths[i],r->needle,r->logPtr);
        	pthread_mutex_lock(&mutex);
        	r->numberOfStrings+=lResult.numberOfStrings;
        	r->numberOfLines+=lResult.numberOfLines;
        	pthread_mutex_unlock(&mutex);
        	pthread_exit(NULL);
    	}	
    }
    return NULL;
}

Result listdir(char *needle, char *directory,FILE *logPtr)
{
	DIR *dptr;
	FILE *fptr;
	int isDirectory;
	struct dirent *direntptr;
	struct stat statbuf;
	char fileOrDirectoryPath[MAX_PATH_SIZE];
	char * file;
	int i;
	int thisPid;
	pthread_t thisFid;
	int counter;
	int fd[2];
	pid_t pid;
	int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    key_t keySM;
    int shmid;
    key=getpid();
    keySM=getpid()*2;

	pthread_mutex_init(&mutex,NULL);

	dptr= opendir(directory);
	if(dptr==NULL)
	{
		fprintf(stderr,"Failed to open directory\n");
		exit(-1); // Error -1
	}

	pipe(fd);
	int fileCount=0;
	int directoryCount=0;
	thisPid=getpid(); // Storing parent pid

	ThreadResult tResult;
	tResult.needle=needle;
	tResult.logPtr=logPtr;
	tResult.numberOfStrings=0;
	tResult.numberOfLines=0;
	while((direntptr = readdir(dptr))!=NULL)
	{
		if (stat(directory,&statbuf)!=stat(direntptr->d_name,&statbuf)) // . and .. ignoreing
		{
			file=direntptr->d_name;
			strcpy(fileOrDirectoryPath,directory);
			strcat(fileOrDirectoryPath,"/");
			strcat(fileOrDirectoryPath,file);
			stat(fileOrDirectoryPath,&statbuf);
			if(!S_ISDIR(statbuf.st_mode)) // Checking if path is file
			{
				fileCount++;
			}
			else // Checking if path is directory
			{
				directoryCount++;
			}
		}
	}
	closedir(dptr);
	tResult.tid=malloc(sizeof(pthread_t)*fileCount);
	tResult.paths=malloc(sizeof(char*)*fileCount);
	for(i=0;i<fileCount;i++)
		tResult.paths[i]=malloc(sizeof(char)*255);
	tResult.numberOfFiles=fileCount;

	if(directoryCount!=0)
	{
		if ((msqid = msgget(key, msgflg )) < 0) {
       		perror("msgget");
        	exit(1);
 	   }
     		//(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
	}

	/*
	ListResult *threadResult;
	if(fileCount!=0)
	{
		int size=sizeof(ListResult);
	     
	    if ((shmid = shmget(keySM, size, IPC_CREAT | 0666)) < 0) {
	        perror("shmget");
	        exit(1);
	    }

	    /*
	     * Now we attach the segment to our data space.
	     
	    if ((threadResult = shmat(shmid, NULL, 0)) == (ListResult *) -1) {
	        perror("shmat");
	        exit(1);
	    }
	    threadResult->numberOfStrings=0;
	    threadResult->numberOfLines=0;
	}

	*/
	dptr= opendir(directory);
	if(dptr==NULL)
	{
		fprintf(stderr,"Failed to open directory\n");
		exit(-1); // Error -1
	}

	fileCount=0;
	directoryCount=0;
	while((direntptr = readdir(dptr))!=NULL)
	{
		if(thisPid==getpid())
		{
			if (stat(directory,&statbuf)!=stat(direntptr->d_name,&statbuf)) // . and .. ignoreing
			{
				file=direntptr->d_name;
				strcpy(fileOrDirectoryPath,directory);
				strcat(fileOrDirectoryPath,"/");
				strcat(fileOrDirectoryPath,file);
				stat(fileOrDirectoryPath,&statbuf);
				if(!S_ISDIR(statbuf.st_mode)) // Checking if path is file
				{
					strcpy(tResult.paths[fileCount],fileOrDirectoryPath);
					fileCount++;
				}
				else // Checking if path is directory
				{
					directoryCount++;
					pid=fork();
				}
			}
		}	
	}

	Result result;
	result.numberOfFiles=fileCount;
	result.maxOfThreads=fileCount;
	result.numberOfDirectories=directoryCount;
	result.numberOfLines=0;
	result.numberOfStrings=0;
	if(fileCount!=0)
		result.cascadeThreads=1;
	else
		result.cascadeThreads=0;

	if(thisPid==getpid())
	{
		ThreadResult* rptr=&tResult;
		for(i=0;i<fileCount;i++)
		{
			//printf("\nFileCount:%d Files:: %s\n",i,result.paths[i]);
			int err = pthread_create(&(tResult.tid[i]), NULL, &function,rptr);
	        if (err != 0)
	            printf("\ncan't create thread :[%s]", strerror(err));
		}
	}
		
	
	if(getpid()!=thisPid) // Child proccess
	{
		Result r=listdir(needle,fileOrDirectoryPath,logPtr); // Then recursive
		//childDir(fileOrDirectoryPath,msqid,r);
		Message sbuf;
		sbuf.mtype = 1;
	    sbuf.result.numberOfLines=r.numberOfLines;
	    sbuf.result.numberOfStrings=r.numberOfStrings;
	    sbuf.result.numberOfDirectories=r.numberOfDirectories;
	    sbuf.result.numberOfFiles=r.numberOfFiles;
	    sbuf.result.maxOfThreads=r.maxOfThreads;
	    sbuf.result.cascadeThreads=r.cascadeThreads;
	    if (msgsnd(msqid, &sbuf, sizeof(sbuf.result), IPC_NOWAIT) < 0) {
	        perror("msgsnd");
	        exit(1);
    	}
		closedir(dptr);	
		exit(0); // exit
	}
	else // Parent proccess
	{
		for(i=0;i<fileCount;i++)
		{
			pthread_join(tResult.tid[i],NULL);	
		}
		result.numberOfStrings=tResult.numberOfStrings;
		result.numberOfLines=tResult.numberOfLines;
		while (wait(NULL) > 0)
  		{
			//Child executed  	 		
 		}
 		// Now parent
 		Result *rptr=&result;
 		//parent(msqid,rptr,directoryCount);
		Message rbuf;
		int cascadeCount=0;
		for(i=0;i<directoryCount;i++)
		{
			if (msgrcv(msqid, &rbuf, sizeof(rbuf.result), 1, 0) < 0) {
       		perror("msgrcv");
        	exit(1);
   			 }
		    /*
		     * Print the answer.
		     */
		    
			result.numberOfStrings+=rbuf.result.numberOfStrings;
			result.numberOfLines+=rbuf.result.numberOfLines;
			result.numberOfFiles+=rbuf.result.numberOfFiles;
			result.numberOfDirectories+=rbuf.result.numberOfDirectories;
			if(result.maxOfThreads<rbuf.result.maxOfThreads)
				result.maxOfThreads=rbuf.result.maxOfThreads;
			if(cascadeCount<rbuf.result.cascadeThreads)
				cascadeCount=rbuf.result.cascadeThreads;
		}
		if(directoryCount!=0)
			result.cascadeThreads+=cascadeCount;
 		closedir(dptr);
 		for(i=0;i<fileCount;i++)
 			free(tResult.paths[i]);
 		free(tResult.paths);
 		free(tResult.tid);
 		if(directoryCount==0)
 			msgctl(msqid,IPC_RMID,NULL);
		return result;
	}
	
}

void childDir(char* directory,int msqid,Result r)
{
	/*
	Message msg;
	msg.mtype=1;
	msg.result=r;
	printf("(Dir:%s Pid:%d):Results are:line:%d\n string:%d\n files:%d\n",directory,getpid(),msg.result.numberOfLines,msg.result.numberOfStrings,msg.result.numberOfFiles);
	msgsnd(msqid,&msg,sizeof(Result),0); */
}

void parent(int msqid,Result* mainResult,int directoryCount)
{
	/*
	int i;
	int deneme=0;
	int counter=0;
	Message msg;
	for(i=0;i<directoryCount;i++)
	{
		msgrcv(msqid,&msg,sizeof(Result),1,0);
		mainResult->numberOfStrings+=msg.result.numberOfStrings;
		mainResult->numberOfLines+=msg.result.numberOfLines;
		mainResult->numberOfFiles+=msg.result.numberOfFiles;
		mainResult->numberOfDirectories+=msg.result.numberOfDirectories;
		if(mainResult->maxOfThreads<msg.result.maxOfThreads)
			mainResult->maxOfThreads=msg.result.maxOfThreads;
	}
	if(directoryCount!=0)
	{
		mainResult->cascadeThreads=msg.result.cascadeThreads+1;
	}
	return;*/
}