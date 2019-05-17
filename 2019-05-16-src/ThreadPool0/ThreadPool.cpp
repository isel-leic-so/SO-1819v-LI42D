#include "stdafx.h"
#include <stdlib.h>

#include "../Include/Threadpool.h"
#include "../Include/BQueue.h"

#define NTHREADS 16

// thread pool globals
static BQUEUE queue;
static HANDLE threads[NTHREADS];

typedef struct  {
	WUNIT_FUNC func;
	LPVOID arg;
} WORK_UNIT, *PWORK_UNIT;

static PWORK_UNIT createWUnit(WUNIT_FUNC f, LPVOID arg) {
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
		PWORK_UNIT u = (PWORK_UNIT)BQ_Get(&queue);
		u->func(u->arg);

		destroyWUnit(u);
	}
	return 0;
}


VOID TpInit() {
	BQ_Init(&queue);

	for (int i = 0; i < NTHREADS; ++i) {
		threads[i] = CreateThread(NULL, 0, WorkerThreadFunc, NULL, 0, NULL);
	}
}

VOID TpQueueItem(WUNIT_FUNC func, LPVOID arg) {
	PWORK_UNIT u = createWUnit(func, arg);
	BQ_Put(&queue, u);
}

BOOL TpOk() {
	return  BQ_IsEmpty(&queue);
}