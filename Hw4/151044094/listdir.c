#include "dirent.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "listdir.h"
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



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *function(void *arg)
{
	unsigned long i = 0;
    pthread_t id = pthread_self();
    ThreadResult *r=arg;

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
	result.numberOfStrings=0;
	result.cascadeThreads=1;

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
		childDir(fd,r);
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
 		parent(fd,rptr,directoryCount);
 		closedir(dptr);
 		for(i=0;i<fileCount;i++)
 			free(tResult.paths[i]);
 		free(tResult.paths);
 		free(tResult.tid);
		return result;
	}
	
}

void childDir(int *fd,Result r)
{
	close(fd[0]);
	write(fd[1],&r,sizeof(r));
	close(fd[1]);
}

void parent(int *fd,Result* mainResult,int directoryCount)
{
	int i;
	int deneme=0;
	int counter=0;
	Result r;
	close(fd[1]);
	for(i=0;i<directoryCount;i++)
	{
		read(fd[0],&r,sizeof(r));
		mainResult->numberOfStrings+=r.numberOfStrings;
		mainResult->numberOfLines+=r.numberOfLines;
		mainResult->numberOfFiles+=r.numberOfFiles;
		mainResult->numberOfDirectories+=r.numberOfDirectories;
		if(mainResult->maxOfThreads<r.maxOfThreads)
			mainResult->maxOfThreads=r.maxOfThreads;
	}
	if(directoryCount!=0)
	{
		mainResult->cascadeThreads=r.cascadeThreads+1;
	}

	close(fd[0]);
	return;
}