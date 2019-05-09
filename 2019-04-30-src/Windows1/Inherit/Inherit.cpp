// Inherit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

HANDLE LaunchProcess(CHAR *cmdLine) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	if (!CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		return NULL;
	}
	
	CloseHandle(pi.hThread);
	return pi.hProcess;
}

VOID Parent() {
	char buffer[] = "parent!\r\n";

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor=NULL;

	HANDLE outFile = CreateFileA("log.txt",FILE_APPEND_DATA,
								0,
								&sa,
		                        CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD written;
	CHAR cmd[128];
	printf("Parent handle:%d\n", (DWORD) outFile);
	sprintf_s(cmd, "Inherit.exe %d", (DWORD) outFile);
	HANDLE child = LaunchProcess(cmd);
	if (!WriteFile(outFile,buffer,strlen(buffer),&written, NULL))
		printf("Error %d writing on handle\n", GetLastError());
	WaitForSingleObject(child, INFINITE);

	CloseHandle(outFile);
 
}


VOID Child(CHAR* argv[]) {
	DWORD written;
	HANDLE handle = (HANDLE)atoi(argv[1]);
	printf("Child: write on handle %d\n", (DWORD) handle);
	char buffer[] = "child!\r\n";
	if (!WriteFile(handle, buffer, strlen(buffer), &written, NULL))
		printf("Error %d writing on handle\n", GetLastError());
}


int main(int argc, CHAR* argv[])
{
	if (argc == 1) 
		Parent();
	else {
		Child(argv);
	}
	
	return 0;
}

