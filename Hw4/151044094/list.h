#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
struct listResult
{
	int numberOfLines;
	int numberOfStrings;
};
typedef struct listResult ListResult;

//takes a char array and fills it.This function returns haySize
// if there is an error returns -1
ListResult readingFromFile(char * fileName,char * needle,int needleSize,FILE *logPtr);

//takes a needle and search it through hay.
// returns how many needle has hay array. If any found returns 0
ListResult findNeedle(FILE *fptr,int charCount,char *needle,int needleSize,FILE *logPtr,char *filename);

//takes a filename and needle. Contains two functions: readingFromFile
// and findNeedle.
ListResult list(char * fileName,char *needle,FILE *logPtr);

#endif	