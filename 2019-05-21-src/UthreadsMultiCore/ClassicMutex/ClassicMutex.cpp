// ClassicMutex.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <tchar.h>
#include <Windows.h>
#include <assert.h>

volatile BOOL interested[2];
volatile int turn;
volatile ULONG val;

VOID EnterCritical(int i) {

	interested[i] = TRUE;
	turn = 1 - i;


	MemoryBarrier();
	// if we do an interlockIncrement here we have the same effect
	//InterlockedIncrement(&val);
	while (interested[1 - i] && turn == 1 - i);
	 
}

VOID LeaveCritical(int i) {
	interested[i] = FALSE;
 
}

#define NTRIES 1000000
DWORD count;

DWORD WINAPI CounterThread(VOID *arg) {
	int id = (DWORD)arg;
	int i = 0;

	//_tprintf(_T("ID=%d\n"), id);
	for (i = 0; i < NTRIES; ++i) {
		EnterCritical(id);

		count += 1;

		LeaveCritical(id);
	}
	return 0;
}



int _tmain(int argc, _TCHAR* argv[])
{

	while (TRUE) {
		HANDLE t1 = CreateThread(NULL, 0, CounterThread, (VOID*)0, 0, NULL);
		HANDLE t2 = CreateThread(NULL, 0, CounterThread, (VOID*)1, 0, NULL);
		SetThreadAffinityMask(t1, 1);
		SetThreadAffinityMask(t2, 2);
		WaitForSingleObject(t1, INFINITE);
		WaitForSingleObject(t2, INFINITE);
		CloseHandle(t1);
		CloseHandle(t2);

		_tprintf(_T("Expected = %d, Real=%d\n"), NTRIES * 2, count);
		//assert(NTRIES*2 == count);
		count = 0;
	}

	return 0;
}

