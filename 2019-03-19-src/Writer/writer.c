/*------------------------------------------------------------
  File mapping using for shared memory.
  This is the writer process.

  JM, 2015
----------------------------------------------------------------*/ 

#include "windows.h"
#include <stdio.h>

#define REGION_SIZE (1024*1024*512)

int  main(int argc, char* argv[])
{
	HANDLE Map=NULL;
	int *mem=NULL;
	int error=0, number;
  
	if (argc != 2) {
		printf("Usage: writer <number>\n");
		return 1;
	}
	number = atoi(argv[1]);
	
	/* Passing INVALID_HANDLE_VALUE for file handle means using the paging file
	   for backing up memory pages.
	   Section object has a name("sharedint") to permit the use by other processes.
	   See the CreateFileMapping explanation in MSDN for more details
	*/
	if ((Map = CreateFileMapping(INVALID_HANDLE_VALUE,
				          NULL, 
						  PAGE_READWRITE,
						  0,
						  REGION_SIZE,
						  "sharedint")) == NULL) {
		error= -1;
		goto terminate;
	}
	

	if ((mem = (LPINT) MapViewOfFile(Map, 
						FILE_MAP_WRITE,
						0,
						0,
						0)) == NULL) {
			error= -2;
			goto terminate;
	}
	printf("Memory mapped at %p!\n", mem);
	
	/* writing shared memory! */
	*mem = number; 
	printf("write %d to mem\n", number);
	
	printf("press enter to terminate!\n");
	getchar();
terminate:

	if (error) 
		printf("error %d using shared memory, system error=%d", error, GetLastError());
	
	if (mem!=NULL)	UnmapViewOfFile(mem);
	if (Map != NULL) CloseHandle(Map);
	
	
	return error;
	
	return 0;
}

