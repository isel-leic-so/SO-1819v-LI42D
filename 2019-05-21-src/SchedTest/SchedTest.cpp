// SchedTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef  unsigned(__stdcall *thread_func)(void *);

#define CACHE_ALIGN 128
volatile  __int64 __declspec(align(CACHE_ALIGN))  LowPriorityCounter;
 
volatile __int64 __declspec(align(CACHE_ALIGN)) HighPriorityCounter1;

volatile __int64 __declspec(align(CACHE_ALIGN)) HighPriorityCounter2;
 
volatile BOOL __declspec(align(CACHE_ALIGN)) testEnd;

HANDLE CreateSuspendedThreadAtPriority(thread_func func, DWORD priority, LPVOID arg)
{
	HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, func,
		arg, CREATE_SUSPENDED, NULL);
	if (thread != NULL) SetThreadPriority(thread, priority);
	
	return thread;

}
 

UINT WINAPI LowPriorityThread(LPVOID arg) {
	while (!testEnd) {
		LowPriorityCounter++;
	
	}
	return 0;
}

UINT WINAPI HighPriorityThread(LPVOID arg)
{
	int id = (INT)arg;
	while (!testEnd)  {
		if (id == 1) HighPriorityCounter1++;
		else HighPriorityCounter2++;
		//printf("High Priority (%d)!\n", GetThreadPriority(GetCurrentThread()));
	}
	 
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE lowPriorityThread, highPriorityThread, highPriorityThread2;
	
	SetProcessAffinityMask(GetCurrentProcess(), 1);


	lowPriorityThread =
		CreateSuspendedThreadAtPriority(LowPriorityThread, THREAD_PRIORITY_LOWEST, NULL);
	 
	highPriorityThread = 
		CreateSuspendedThreadAtPriority(HighPriorityThread, THREAD_PRIORITY_TIME_CRITICAL,
			(LPVOID) 1);
 
	highPriorityThread2 =
		CreateSuspendedThreadAtPriority(HighPriorityThread, THREAD_PRIORITY_TIME_CRITICAL,
			(LPVOID)2);

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	 
	ResumeThread(lowPriorityThread);
	ResumeThread(highPriorityThread);
	ResumeThread(highPriorityThread2);
	
	Sleep(5020);
	testEnd = TRUE;
	
	WaitForSingleObject(lowPriorityThread, INFINITE);
	WaitForSingleObject(highPriorityThread, INFINITE);
	WaitForSingleObject(highPriorityThread2, INFINITE);

	CloseHandle(lowPriorityThread);
	CloseHandle(highPriorityThread);

	printf("LowPriorityCounter: %I64d, HighPriorityCounters: (%I64d, %I64d)\n",
		LowPriorityCounter, HighPriorityCounter1, HighPriorityCounter2);
	return 0;
}

