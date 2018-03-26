#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>

//takes a char array and fills it.This function returns haySize
// if there is an error returns -1
int readingFromFile(char * fileName,char * needle,int needleSize,FILE *logPtr);

//takes a needle and search it through hay.
// returns how many needle has hay array. If any found returns 0
int findNeedle(FILE *fptr,int charCount,char *needle,int needleSize,FILE *logPtr,char *filename);

//takes a filename and needle. Contains two functions: readingFromFile
// and findNeedle.
int list(char * fileName,char *needle,FILE *logPtr);

#endif	