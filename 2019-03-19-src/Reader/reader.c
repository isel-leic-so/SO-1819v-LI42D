/*------------------------------------------------------------
  File mapping using for shared memory.
  This is the reader process.

  JM,2015
----------------------------------------------------------------*/

#include "windows.h"
#include <stdio.h>


#define REGION_SIZE (1024*1024*256)



int  main(int argc, char* argv[])
{
	int res=0;
	HANDLE Map=NULL;
	int *mem=NULL;
	 
	/* Passing INVALID_HANDLE_VALUE for file handle means using the paging file
	   for  memory pages backing store.
	   Section object has a name("sharedint") to permit the use by other processes.
	   See the CreateFileMapping explanation in MSDN for more details.
	*/
	if ((Map = CreateFileMapping(INVALID_HANDLE_VALUE,
				          NULL, 
						  PAGE_READONLY,
						  0,
						  REGION_SIZE,
						  "sharedint")) == NULL) {
		res= -1;
		goto terminate;
	}
	

	if ((mem = (LPINT) MapViewOfFile(Map, 
						FILE_MAP_READ,
						0,
						0,
						sizeof(int))) == NULL) {
			res= -2;
			goto terminate;
	}
	printf("Memory mapped at %p!\n", mem);

	/* reading shared memory! */
	res = *mem; 
	
terminate:
	if (res < 0)
		printf("error %d using shared memory, system error=%d", res, GetLastError());
	else
		printf("read %d from shared memory\n", res);
	if (mem!=NULL)	UnmapViewOfFile(mem);
	if (Map != NULL) CloseHandle(Map);
	return res;
 
}

