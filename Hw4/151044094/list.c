#include "list.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

ListResult list(char * fileName,char *needle,FILE *logPtr)
{
	int CAPACITY;
	int needleSize;
	int needleCount;
	ListResult lResult;
	// Finding size of string we want to search
	needleSize=0;
	while(needle[needleSize]!='\0')
	{
		needleSize++;
	}
	lResult=readingFromFile(fileName,needle,needleSize,logPtr);

	return lResult;
}

ListResult readingFromFile(char * fileName,char * needle,int needleSize,FILE *logPtr)
{
	FILE *fptr;
	int i;
	int charCount;
	ListResult lResult;
	lResult.numberOfStrings=0;
	lResult.numberOfLines=0;

	fptr=fopen(fileName,"r");
	if(fptr==NULL)
	{
		fprintf(stderr,"Error while opening file\n");
		return lResult; // Error -1
	}
	fseek(fptr, 0, SEEK_END);
	charCount=ftell(fptr);
	if(charCount==0)
	{
		return lResult; // File is empty
	}
	fseek(fptr, 0, SEEK_SET);

	lResult=findNeedle(fptr,charCount,needle,needleSize,logPtr,fileName);
	fclose(fptr); // Closing fileName
	return lResult;
}

ListResult findNeedle(FILE *fptr,int charCount,char *needle,int needleSize,FILE *logPtr,char *filename)
{
	char data;
	int row,col,counter;
	int seekPoint;
	int trigger;
	int i,k;
	ListResult lResult;

	counter=0;
	col=1; //(1,1) is start point
	row=1;
	for(i=0;i<charCount;++i)
	{
		data=fgetc(fptr);
		if(data==needle[0])
		{
			seekPoint=ftell(fptr);
			trigger=1;
			for(k=1;k<needleSize;)
			{
				data=fgetc(fptr);
				if(needle[k]!=data)
				{
					if(data==' '|| data=='	' || data=='\n')
					{
						// Bilerek boş bırakılmıştır.
					}
					else
					{
						trigger=0;
						k++;
					}
				}

				else
				{
					k++;
				}
			}
			
			if(trigger==1)
			{
				//printf("%s:  [%d,%d]  %s  first character is found.\n",filename,row,col,needle);
				fprintf(logPtr,"%li - %d %s:  [%d,%d]  %s  first character is found.\n",pthread_self(),getpid(),filename,row,col,needle);
				counter++;
			}
			fseek(fptr,seekPoint,SEEK_SET); // Nerede bu ife girdiysek oraya
			i=seekPoint-1; // Oraya geri dönüyoruz ve
			col++; // column sayısını arttırıyoruz
		}

		else if(data=='\n')
		{
			col=1;
			row++;
		}
		else
		{
			col++;
		}
	}
	lResult.numberOfStrings=counter;
	lResult.numberOfLines=row;
	return lResult;

}