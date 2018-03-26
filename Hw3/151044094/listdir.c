#include "dirent.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "list.h"
#include "listdir.h"
#include <fcntl.h>	
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_PATH_SIZE 255
#define FIFO_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define FIFO_MODES (O_RDONLY|O_WRONLY)


int listdir(char *needle, char *directory,FILE *logPtr)
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
	int counter;
	int fd[2];
	int fDir;
	int fdirectory[2];
	char * myfifo="fifo";
	pid_t pid;

	dptr= opendir(directory);
	if(dptr==NULL)
	{
		fprintf(stderr,"Failed to open directory\n");
		exit(-1); // Error -1
	}

	if(mkfifo(myfifo,FIFO_PERM)<0)
	{
		if(errno!=EEXIST)
		{
			fprintf(stderr,"(%ld):failed to create named pipe %s: %s\n",(long)getpid(),myfifo,strerror(errno));
			exit(-1);
		}
	}
	fptr=fopen(myfifo,"a+"); // This file pointer is never used!

	pipe(fd);
	pipe(fdirectory);
	thisPid=getpid(); // Storing parent pid
	isDirectory=1; // Default is directory because of our argv[2] is directory
	while((direntptr = readdir(dptr))!=NULL)
	{
		if (stat(directory,&statbuf)!=stat(direntptr->d_name,&statbuf)) // . and .. ignoreing
		{
			if(getpid()==thisPid)
			{
				pid=fork();
				if(pid==0)
				{
  					file=direntptr->d_name;
  					strcpy(fileOrDirectoryPath,directory);
					strcat(fileOrDirectoryPath,"/");
					strcat(fileOrDirectoryPath,file);
					stat(fileOrDirectoryPath,&statbuf);
					if(!S_ISDIR(statbuf.st_mode)) // Checking if path is file
					{
						isDirectory=0;
						close(fdirectory[0]);
						close(fdirectory[1]);
					}
					else // This block adds 1 directory to pipe
					{
						close(fdirectory[0]);
						int dirTrigger=1;
						if(write(fdirectory[1],&dirTrigger,sizeof(int))<0)
						{
							fprintf(stderr,"(%ld):failed to write pipe: %s\n",(long)getpid(),strerror(errno));
							exit(-1);
						}
						close(fdirectory[1]);
					}
				}
				else if(pid<0)
				{
					fprintf(stderr,"Error while forking!\n");
					exit(-1);
				}
			}
		}	
	}

	counter=0;
	if(getpid()!=thisPid) // Child proccess
	{
		if(isDirectory==1) // Checking if path is directory
		{
			counter=listdir(needle,fileOrDirectoryPath,logPtr); // Then recursive
			if(counter==-1) // If return value is error
				counter=0;
			childDir(myfifo,counter);

			// Closing pipes
			close(fd[1]);
			close(fd[0]);
		}

		else // Checking if path is file
		{
			counter=list(fileOrDirectoryPath,needle,logPtr); // Then call list function
			if(counter==-1)
				counter=0;
			childFile(fd,counter);
		}

		// Closing pipes
		closedir(dptr);
		fclose(fptr);
		exit(0); // exit
	}
	else // Parent proccess
	{
		while (wait(NULL) > 0)
  		{
			//Child executed  	 		
 		}
 		// Now parent
 		int result=parent(fd,fdirectory,myfifo);
 		
		closedir(dptr);
		fclose(fptr);
		return result;
	}
	
}

void childFile(int *fd,int count)
{
	close(fd[0]);
	if(write(fd[1],&count,sizeof(int))<0)
	{
		fprintf(stderr,"%ld:(child) failed to write pipe %s\n",(long)getpid(),strerror(errno));
		exit(-1);
	}
	close(fd[1]);
}

void childDir(char *myfifo,int count)
{
	int fDir;
	while(((fDir=open(myfifo,O_WRONLY|O_NONBLOCK))==-1)&&(errno==EINTR));
	if(fDir==-1)
	{
		fprintf(stderr,"%ld:(child) failed to open named pipe %s for write %s\n",(long)getpid(),myfifo,strerror(errno));
		exit(-1);
	}
	int no=write(fDir,&count,sizeof(int));
	if(no!=sizeof(int))
	{
		fprintf(stderr,"%ld:(child) failed to write named pipe %s for write %s\n",(long)getpid(),myfifo,strerror(errno));
		exit(-1);
	}
	close(fDir);
}

int parent(int *fd,int *fdirectory,char * myfifo)
{
	int fDir;
	int i;
	int deneme=0;
	int counter=0;

	// Parent communicates with file children with pipe
	close(fd[1]);
	while(read(fd[0],&deneme,sizeof(int)))
	{
		counter+=deneme;
	}
	close(fd[0]);

	// Directory is not communicate with pipe
	// This pipe only tells how many directory parent has
	close(fdirectory[1]);
	int directoryNumber;
	int directoryCount=0;
	while(read(fdirectory[0],&directoryNumber,sizeof(int)))
	{
		directoryCount+=directoryNumber;
	}
	close(fdirectory[0]);

	// Parent communicates with directory children with FIFO
	while(((fDir=open(myfifo,O_RDONLY))==-1)&&(errno==EINTR));
	if(fDir==1)
	{
		fprintf(stderr,"%ld:(parent) failed to open named pipe %s for read %s\n",(long)getpid(),myfifo,strerror(errno));
		exit(-1);
	}
	
	int a;
	for(i=0;i<directoryCount;i++)
	{
		read(fDir,&deneme,sizeof(int));
		counter+=deneme;
	}
	close(fDir);
	return counter;
}