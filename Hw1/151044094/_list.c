#include "list.h"
#include <stdio.h>
#include <stdlib.h>

int list(char * fileName,char *needle)
{
	int CAPACITY;
	int needleSize;
	char * hay; // all the lines
	int haySize;
	int needleCount;
	
	CAPACITY=50;
	hay=malloc(CAPACITY*sizeof(char));
	haySize=readingFromFile(fileName,hay,CAPACITY);
	if(haySize==-1)
	{
		return -1; // Error -1
	}	
	
	// Finding size of string we want to search
	needleSize=0;
	while(needle[needleSize]!='\0')
	{
		needleSize++;
	}

	needleCount=findNeedle(needle,needleSize,hay,haySize);
	// Finding needles in hay
	printf("Total number of %s is: %d in file %s\n",needle,needleCount,fileName);

	free(hay); //Freeing memory
	return needleCount;
}

int readingFromFile(char * fileName,char * hay,int cap)
{
	FILE *fptr;	
	int i;

	fptr=fopen(fileName,"r");
	if(fptr==NULL)
	{
		printf("Error while opening file\n");
		return -1; // Error -1
	}
	
	// Reading from file
	char data;
	i=0;
	while(fread(&data,sizeof(char),1,fptr))
	{
		if(i==cap) // If capacity is full
		{
			cap*=2;
			hay=realloc(hay,cap*sizeof(char)); //Getting more memory for line
		}
		hay[i]=data;
		i++;
	}
	fclose(fptr); // Closing file
	return i;
}

int findNeedle(char *needle,int needleSize,char *hay,int haySize)
{
	int row,col,counter;
	int trigger;
	int i,l,k;

	counter=0;
	col=1; //(1,1) is start point
	row=1;
	for(i=0;i<haySize-needleSize+1;++i)
	{
		if(hay[i]==needle[0])
		{
			trigger=1;
			l=i+1;
			for(k=1;k<needleSize;)
			{
				if(needle[k]!=hay[l])
				{
					if(hay[l]==' '|| hay[l]=='	' || hay[l]=='\n')
					{
						l++;
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
					l++;
				}
			}
			if(trigger==1)
			{
				printf("%s found at (%d,%d) (col,row)\n",needle,col,row);
				counter++;
			}
		}

		if(hay[i]=='\n')
		{
			col++;
			row=0;
		}
		else
		{
			row++;
		}
	}
	return counter;

}