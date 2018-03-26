#include "dirent.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "unistd.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_PATH_SIZE 255

int listdir(char *needle, char *directory,FILE *logPtr);

int listdir(char *needle, char *directory,FILE *logPtr)
{
	DIR *dptr;
	FILE *fptr;

	struct dirent *direntptr;
	struct stat statbuf;
	char fileOrDirectoryPath[MAX_PATH_SIZE];
	char sharedMemoryFile[MAX_PATH_SIZE];
	char * file;
	int filesCapacity=10;
	int fileCount;
	int i;
	int thisPid;
	int counter;
	pid_t pid;

	dptr= opendir(directory);
	if(dptr==NULL)
	{
		printf("Failed to open directory\n");
		return -1; // Error -1
	}

	thisPid=getpid();
	i=0;
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
				}
				else if(pid<0)
				{
					fprintf(stderr,"Error while forking!\n");
					return -1;
				}
			}
		}	
	}
	if(i=0)
	{
		fprintf(stderr,"This directory has no file or directory.\n");
		return 0;
	}
	strcpy(sharedMemoryFile,directory); 
	strcat(sharedMemoryFile,"/");
	strcat(sharedMemoryFile,"temp.txt");
	fptr=fopen(sharedMemoryFile,"a+");


	int deneme=0;
	counter=0;
	if(getpid()!=thisPid) // Child proccess
	{
		strcpy(fileOrDirectoryPath,directory);
		strcat(fileOrDirectoryPath,"/");
		strcat(fileOrDirectoryPath,file);

		stat(fileOrDirectoryPath,&statbuf);
		
		if(S_ISDIR(statbuf.st_mode)) // Checking if path is directory
			deneme=listdir(needle,fileOrDirectoryPath,logPtr); // Then recursive

		else // Checking if path is file
			deneme=list(fileOrDirectoryPath,needle,logPtr); // Then call list function

		if(deneme==-1)
		{
			deneme=0;
		}
		fwrite(&deneme,sizeof(int),1,fptr);
		fclose(fptr);
		closedir(dptr);
		exit(0); // exit
	}
	else // Parent proccess
	{
		while (wait(NULL) > 0)
  		{
			//Child executed  	 		
 		}
 		// Now parent
 		deneme=0;
 		fseek(fptr, 0, SEEK_SET);
 		while(fread(&deneme,sizeof(int),1,fptr))
 		{
 			counter+=deneme;
 		}
 		fclose(fptr);
 		closedir(dptr);
 		remove(sharedMemoryFile); // Delete shared memory file

		return counter;
	}
	
}