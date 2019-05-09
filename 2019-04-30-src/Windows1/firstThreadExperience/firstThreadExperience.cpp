// firstThreadExperience.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

DWORD WINAPI ThreadFunc(PVOID arg) {
	Sleep(4000);
	printf("Child thread is is %d\n", GetCurrentThreadId());
	return 10;
}

int main()
{
	DWORD tid;

	printf("Parent thread is is %d\n", GetCurrentThreadId());
	HANDLE hThread = CreateThread(
		NULL,
		0,
		ThreadFunc,
		NULL,
		0,
		&tid);
	printf("Nova thread criada com o id %d\n", tid);
	WaitForSingleObject(hThread, INFINITE);
	DWORD threadCode;

	
	GetExitCodeThread(hThread, &threadCode);
	CloseHandle(hThread);
	printf("Thread exit with code  %d\n", threadCode);
    return 0;
}

