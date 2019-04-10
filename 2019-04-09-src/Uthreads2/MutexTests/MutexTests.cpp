// MutexTests.cpp : Defines the entry point for the console application.
//

/////////////////////////////////////////////
//
// CCISEL 
// 2007-2010
//
// UThread    Mutex    Test
//
// Jorge Martins, 2014
////////////////////////////////////////////

#include "stdafx.h"


#include "../Include/Uthread.h"
#include "../Include/USynch.h"

#define NITERS 10000
#define NTHREADS 20

//#define WITH_MUTEX

typedef struct Args {
	DWORD *result;
#pragma region Atomic
#ifdef WITH_MUTEX
	PMUTEX mutex;
#endif
#pragma endregion;
} ARGS, *PARGS;

VOID CounterThread(UT_ARGUMENT Argument) {
	PARGS args = (PARGS)Argument;
	ULONG Index;
	 
	for (Index = 0; Index < NITERS; ++Index) {
		DWORD tmp;

#pragma region Atomic
#ifdef WITH_MUTEX
		MutexAcquire(args->mutex);
#endif
#pragma endregion	
		
		tmp = *args->result + 1;
#define TROUBLE
#ifdef TROUBLE
		if ((rand() % 32) == 0)
			UtYield();
#endif
		*args->result = tmp;
#pragma region Atomic
#ifdef WITH_MUTEX
		MutexRelease(args->mutex);
#endif  
#pragma endregion
	}
}


void Test2() {
	int Index;
	DWORD res, result = 0;
#ifdef WITH_MUTEX
	MUTEX mutex;
#endif
	ARGS args;


	srand(GetTickCount());
#pragma region Atomic
#ifdef WITH_MUTEX
	MutexInit(&mutex, FALSE);
	args.mutex = &mutex;
#endif
#pragma endregion

	args.result = &result;
	
	
	for (Index = 0; Index < NTHREADS; ++Index) {
		UtCreate(CounterThread, &args);
	}

	 
	UtRun();
	res = result;

	 
	if (result == NITERS * NTHREADS)
		printf("test succeeded, result=%d!\n", result);
	else
		printf("counting error, result=%d!\n", result);
}


int main() {
	while (TRUE) {
		UtInit();

		Test2();
		UtEnd();
	}
	return 0;
}

