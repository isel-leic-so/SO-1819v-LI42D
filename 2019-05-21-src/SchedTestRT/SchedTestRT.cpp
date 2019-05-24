// SchedTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

TCHAR sharedname[] = _T("SchedTestSharedMem");

typedef  unsigned(__stdcall *thread_func)(void *);

typedef struct _shared {
	volatile BOOL endFlag;
	volatile __int64 LowPriorityCounter;
	volatile __int64 HighPriorityCounter;
	volatile BOOL timeout;
} SHARED_STATE, *LPSHARED_STATE;

HANDLE lowPriorityThread, highPriorityThread;
LPSHARED_STATE global;

HANDLE LaunchProcess(TCHAR *cmdLine, DWORD priorityClass, HANDLE *primaryThread) {
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);


	if (!CreateProcess(NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		priorityClass | CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi)) {
		return NULL;
	}

	*primaryThread = pi.hThread;
	return pi.hProcess;
}


HANDLE CreateSharedRegion()
{	
	
	HANDLE mem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
		0, sizeof(SHARED_STATE), sharedname);
	if (mem == NULL)
	{
		_tprintf(_T("Error %d creating shared memory object\n"), GetLastError());
		return FALSE;
	}
	if ((global = (LPSHARED_STATE) MapViewOfFile(mem, FILE_MAP_WRITE, 0, 0, 0)) == NULL) {
		_tprintf(_T("Error %d mapping shared memory\n"), GetLastError());
		CloseHandle(mem);
		return FALSE;
	}
	
	
	return mem;
}

HANDLE CreateSuspendedThreadAtPriority(thread_func func, DWORD priority)
{
	HANDLE thread = (HANDLE) _beginthreadex(NULL, 0, func,
		NULL, CREATE_SUSPENDED, NULL);
	if (thread != NULL) SetThreadPriority(thread, priority);
	return thread;

}

UINT WINAPI WatchDogThread(LPVOID arg)
{
	Sleep(10000);
 
	global->timeout = TRUE;
	global->endFlag = TRUE;
	return 0;
}


UINT WINAPI LowPriorityThread(LPVOID arg)
{
	Sleep(200);
	while (!global->endFlag)
		global->LowPriorityCounter++;
	return 0;
}

UINT WINAPI HighPriorityThread(LPVOID arg)
{
	 
	while (!global->endFlag)   
		global->HighPriorityCounter++;
	 
	return 0;
}



VOID Parent() {
	TCHAR cmdLine[] = _T("SchedTestRT.exe child");
	 
	// Shared memory region
	HANDLE mem = NULL;
	HANDLE lowPriorityThread = NULL;
	HANDLE highPriorityThread = NULL;
	HANDLE childProc = NULL;

	// Force all parent threads running at CPU1
	SetProcessAffinityMask(GetCurrentProcess(), 1);

	// Create shared region
	if ((mem=CreateSharedRegion()) == NULL) return;

	// Create low priority thread
	lowPriorityThread =
		CreateSuspendedThreadAtPriority(LowPriorityThread, THREAD_PRIORITY_IDLE);
	if (lowPriorityThread == NULL) return;

	// Create Child Process at realtime priority class (dangerous!)
	childProc = LaunchProcess(cmdLine, REALTIME_PRIORITY_CLASS, &highPriorityThread);
	if (childProc == NULL || highPriorityThread == NULL) {
		_tprintf(_T("Error %d creating  child process\n"), GetLastError());
		goto end;
		 
	}
	// Force all child threads to run at same CPU as parent (CPU1)
	SetProcessAffinityMask(childProc, 1);
	 
	// start parent and child counter threads
	ResumeThread(highPriorityThread);
	ResumeThread(lowPriorityThread);


	// wait for two seconds
	Sleep(2000);
	
	// Force test end
	global->endFlag = TRUE;
 
	// wait before show results
	WaitForSingleObject(lowPriorityThread, INFINITE);
	WaitForSingleObject(childProc, INFINITE);
	
	_tprintf(_T("Parent: end!\n"));
	printf("LowPriorityCounter: %I64d, HighPriorityCounter: %I64d\n",
		global->LowPriorityCounter, global->HighPriorityCounter);
	printf("timeout? %d\n", global->timeout );
end:
	if (mem != NULL) CloseHandle(mem);
	if (global != NULL) UnmapViewOfFile((LPVOID) global);
	if (childProc != NULL) CloseHandle(childProc);
	if (highPriorityThread != NULL) CloseHandle(highPriorityThread);
	if (lowPriorityThread != NULL) CloseHandle(lowPriorityThread);
}


VOID Child() {
	HANDLE watchDogThread = NULL;
	
	// Create shared region
	if (CreateSharedRegion() == NULL) return;

	// Create Watchdog thread, just in case...
	watchDogThread =
		CreateSuspendedThreadAtPriority(WatchDogThread, THREAD_PRIORITY_TIME_CRITICAL);
	if (watchDogThread == NULL)
		goto end;
	// Start watchDogThread
	ResumeThread(watchDogThread);
	CloseHandle(watchDogThread);

	// Start counting at realtime priority
	HighPriorityThread(NULL);

end:
	if (global != NULL) UnmapViewOfFile((LPVOID)global);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	if (argc > 1) 
		Child();
	else
		Parent();

	return 0;
}

