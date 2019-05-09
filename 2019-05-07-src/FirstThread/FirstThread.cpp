// FirstThread.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

DWORD WINAPI StartFunc(LPVOID arg) {
	Sleep(2000);
	printf("new thread: id %d\n", GetCurrentThreadId());
	return 0;
}

int main()
{
	DWORD threadId;

	HANDLE threadHandle = CreateThread(
		NULL,
		0,
		StartFunc,
		NULL,
		0,
		&threadId);
	if (threadHandle == NULL) {
		printf("error %d creating thread!\n", GetLastError());
		return 1;
	}
	printf("thread created with id %d\n", threadId);
	WaitForSingleObject(threadHandle, INFINITE);
	//Sleep(2500);
	return 0;
}
