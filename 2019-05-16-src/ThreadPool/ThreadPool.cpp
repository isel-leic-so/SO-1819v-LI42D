#include "stdafx.h"
#include <stdlib.h>

#include "../Include/Threadpool.h"
#include "../Include/BQueue.h"

#define NTHREADS 16

// thread pool globals
static BQUEUE queue;
static HANDLE threads[NTHREADS];
static DWORD nThreads;
static DWORD initialized;

typedef struct  {
	LPTHREAD_START_ROUTINE func;
	LPVOID arg;
} WORK_UNIT, *PWORK_UNIT;

static PWORK_UNIT createWUnit(LPTHREAD_START_ROUTINE f, LPVOID arg) {
	PWORK_UNIT n = (PWORK_UNIT)malloc(sizeof(WORK_UNIT));
	n->func = f;
	n->arg = arg;
	return n;
}

static VOID destroyWUnit(PWORK_UNIT n) {
	free(n);
}

static DWORD WINAPI WorkerThreadFunc(LPVOID arg) {
	while (TRUE) {
		PWORK_UNIT item = (PWORK_UNIT)BQ_Get(&queue);

		item->func(item->arg);
		destroyWUnit(item);
	}
}


VOID TpInit() {
	DWORD i = InterlockedExchange(&initialized, 1);
	if (i == 0) {
	 
		BQ_Init(&queue);
		SYSTEM_INFO si;

		GetSystemInfo(&si);

		for (DWORD i = 0; i < si.dwNumberOfProcessors; ++i)
			threads[i] = CreateThread(NULL, 0, WorkerThreadFunc, NULL,
				0, NULL);
	}
	
}

VOID TpQueueItem(LPTHREAD_START_ROUTINE func, LPVOID arg) {
	BQ_Put(&queue, createWUnit(func, arg));
}
