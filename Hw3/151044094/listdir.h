#ifndef LISTDIR_H
#define LISTDIR_H

#include <stdlib.h>
#include <stdio.h>

//takes needle: string we want to search
//takes directory: directory we want to search in
//takes logPtr: log.txt's FILE* to write logs
//returns int: total number of needles in directory
int listdir(char *needle, char *directory,FILE *logPtr);

//takes fd: to communicate with file children
//takes fDir: to get how many directory has this directory
//takes myfifo: to communicate with directory children
//returns int: calculate how many needles has total
int parent(int *fd,int *fDir,char * myfifo);

//takes myfifo: to communicate with parent
//takes count: this count will be written to fifo
void childDir(char *myfifo,int count);

//takes fd: to communicate with file children
//takes count: this count will be written to fifo
void childFile(int *fd,int count);

#endif	
