// Shell.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


HANDLE LaunchProcess(PCHAR cmdLine) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si = { sizeof(si)  };
	si.lpTitle = "myshell";
	if (!CreateProcessA(
		NULL, 
		cmdLine, 
		NULL, 
		NULL, 
		FALSE, 
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL, 
		&si, 
		&pi)) {
		return NULL;
	}
	
	CloseHandle(pi.hThread);
	return pi.hProcess;
}

int main(int argc, char* argv[])
{
	char line[128];

	printf("> ");
	while (gets_s(line, 128) != NULL) {
		if (line[0] != 0)  {
			if (strcmp(line, "exit") == 0)
				break;
			HANDLE p = LaunchProcess(line);
			if (p == NULL)
				printf("Error %d: execing %s\n", GetLastError(), line); 
			else {
				 
				WaitForSingleObject(p, INFINITE);
				DWORD exitCode;
				GetExitCodeProcess(p, &exitCode);
				printf("Process returned %d\n", exitCode);
			 
				CloseHandle(p);
                  
			}
		}
		printf("> ");
	}
	return 0;
}

