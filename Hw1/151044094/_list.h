#ifndef LIST_H
#define LIST_H

//takes a char array and fills it.This function returns haySize
// if there is an error returns -1
int readingFromFile(char * fileName,char *hay,int cap);

//takes a needle and search it through hay.
// returns how many needle has hay array. If any found returns 0
int findNeedle(char *needle,int needleSize,char *hay,int haySize);

//takes a filename and needle. Contains two functions: readingFromFile
// and findNeedle.
int list(char * fileName,char *needle);

#endif	