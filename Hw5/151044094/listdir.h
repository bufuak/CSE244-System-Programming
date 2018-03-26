#ifndef LISTDIR_H
#define LISTDIR_H

#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#define MAXOFSIZES 20
struct threadResult
{
	char *needle;
	FILE *logPtr;
	char **paths;
	pthread_t *tid;
	int numberOfFiles;
	int numberOfStrings;
	int numberOfLines;
}; 
typedef struct threadResult ThreadResult;

struct threadResultSM
{
	int numberOfStrings;
	int numberOfLines;
};
typedef struct threadResultSM ThreadResultSM;

struct msg
{
    long mtype;
    struct result_message
    {
        int numberOfDirectories;
        int numberOfFiles;  
        int maxOfThreads;
        int numberOfStrings;
        int numberOfLines;
        int cascadeThreads;
    }result;
};
typedef struct msg Message;

struct result
{
    int numberOfDirectories;
    int numberOfFiles;  
    int maxOfThreads;
    int numberOfStrings;
    int numberOfLines;
    int cascadeThreads;
};
typedef struct result Result;

//takes needle: string we want to search
//takes directory: directory we want to search in
//takes logPtr: log.txt's FILE* to write logs
//returns int: total number of needles in directory
Result listdir(char *needle, char *directory,FILE *logPtr);

//takes fd: to communicate with file children
//takes fDir: to get how many directory has this directory
//takes myfifo: to communicate with directory children
//returns int: calculate how many needles has total
void parent(int msqid,Result* rptr,int directoryCount);

//takes myfifo: to communicate with parent
//takes count: this count will be written to fifo
void childDir(char* directory,int msqid,Result r);

#endif	
