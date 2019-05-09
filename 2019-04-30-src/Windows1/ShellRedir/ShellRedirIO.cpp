// Shellredirio.cpp : Defines the entry point for the console application.
//
// Variante de shell com possibilidade de redireccionamento de standard output

// Exemplo:
//	# type shellredirio.cpp > out.txt
//	Redirecciona o output standard do comando type para o ficheiro out.txt
//
//  Jmartins, 2018

#include "stdafx.h"
#include <string.h>

INT SplitStr(LPSTR str, CHAR delimiter, char strs[][1024], int maxSplits);

extern HANDLE outFileHandle;
HANDLE LaunchProcess(CHAR *cmdLine, CHAR *outFile);
	 
VOID ProcessEnvCmd(char *line) {
	char parts[3][1024];
	int nparts = SplitStr(line, ' ', parts, 3);
	if (nparts == 3) {
		SetEnvironmentVariableA(parts[1], parts[2]);
	}
}

int main(int argc, char* argv[])
{
	char line[128];
	char parts[2][1024];

	printf("# ");
	while (gets_s(line, 128) != NULL) {
		if (line[0] != 0)  {
			if (strcmp(line, "exit") == 0)
				break;
			if (strstr(line, "setenv") != NULL) {
				ProcessEnvCmd(line);
				printf("# ");
				continue;
			}
	
			int nParts = SplitStr(line, _T('>'), parts, 2);
			 
			HANDLE p = LaunchProcess(parts[0],nParts==1? NULL:parts[1]);
			 
			if (p == NULL)
					printf("Error %d: executing %s\n", GetLastError(), line); 
			else {
				
                //printf("process %d created!\n", GetProcessId(p));
				WaitForSingleObject(p, INFINITE);
				if (outFileHandle != INVALID_HANDLE_VALUE)
					CloseHandle(outFileHandle);
				CloseHandle(p);
                  
			}
		}
		printf("# ");
	}
	return 0;
}

