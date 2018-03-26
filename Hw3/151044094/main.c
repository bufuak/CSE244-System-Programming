#include "stdio.h"
#include "stdlib.h"
#include "listdir.h"

int main(int argc,char * argv[])
{
	int numberOfNeedle;
	char * needle;
	char * path;
	FILE *logPtr;
	logPtr=fopen("log.txt","a+");
	
	// Usage
	if(argc!=3)
	{
		printf("Proper usage of this program is\n./exe <string> <directory>\n");
		return -1;
	}

	needle=argv[1];
	// Checking if the directory exists
	path=argv[2];
	numberOfNeedle=listdir(needle,path,logPtr);
	if(numberOfNeedle==-1)
		return -1;
	else
	{
		printf("Successful! All the log can be found in log.txt\n");
		fprintf(logPtr,"-------------------------------------------------\n");
		fprintf(logPtr,"%d %s were found in total.\n",numberOfNeedle,needle);
	}
	fclose(logPtr);
	return 0;
}