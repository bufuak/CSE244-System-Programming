#include "list.h"
#include <stdio.h>
#include <stdlib.h>

int list(char * fileName,char *needle,FILE *logPtr)
{
	int CAPACITY;
	int needleSize;
	int needleCount;
	printf("%s\n",fileName);
	// Finding size of string we want to search
	needleSize=0;
	while(needle[needleSize]!='\0')
	{
		needleSize++;
	}

	needleCount=readingFromFile(fileName,needle,needleSize,logPtr);

	return needleCount;
}

int readingFromFile(char * fileName,char * needle,int needleSize,FILE *logPtr)
{
	FILE *fptr;
	int i;
	int charCount;

	fptr=fopen(fileName,"r");
	if(fptr==NULL)
	{
		fprintf(stderr,"Error while opening file\n");
		return -1; // Error -1
	}
	fseek(fptr, 0, SEEK_END);
	charCount=ftell(fptr);
	if(charCount==0)
	{
		return 0; // File is empty
	}
	fseek(fptr, 0, SEEK_SET);
	i=findNeedle(fptr,charCount,needle,needleSize,logPtr,fileName);
	fclose(fptr); // Closing fileName
	return i;
}

int findNeedle(FILE *fptr,int charCount,char *needle,int needleSize,FILE *logPtr,char *filename)
{
	char data;
	int row,col,counter;
	int seekPoint;
	int trigger;
	int i,k;

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
				fprintf(logPtr,"%s:  [%d,%d]  %s  first character is found.\n",filename,row,col,needle);
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
	return counter;

}