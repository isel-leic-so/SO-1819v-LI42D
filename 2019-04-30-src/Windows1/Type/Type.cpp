// Type.cpp : Defines the entry point for the console application.
//
// Show on standard output the content of the specified text file
//

#include "stdafx.h"

VOID WindowsType(char *name) {
	HANDLE H = CreateFileA(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

}

int main(int argc, char *argv[])
{
	char line[1024];
	
	if (argc != 2) {
		fprintf(stderr, "usage: type <file>\n");
		return 1;
	}
	FILE* file = NULL;
	 
	if (fopen_s(&file, argv[1], "r") != 0) {
		fprintf(stderr, "can´t open file %s\n", argv[1]);
		return 2;
	}
	
	while (fgets(line, 1024, file) != NULL) {
		printf("%s", line);
		
	}
	fflush(stdout);
	fclose(file);
	 
    return 0;
}

