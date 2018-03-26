#include <stdio.h>
#include "list.h"
int main(int argc,char * argv[])
{
	char * needle;
	char * fileName;

	// Usage:
	if(argc!=3)
	{
		printf("Proper usage of this program is:\n ./list <string> <filename>\n");
		return -1; // Error -1 
	}

	needle=argv[1]; // String we want to search in file
	fileName=argv[2]; // file we want to search string in
	list(fileName,needle);
	return 0;
}