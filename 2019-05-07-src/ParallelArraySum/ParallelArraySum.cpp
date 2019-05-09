// ParallelArraySum.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#define MAX_PARTS 64

typedef struct {
	INT * start;
	INT *end;
	LONG partialCount;
} WorkerCtx, *PWorkerCtx;

DWORD WINAPI WorkerFunc(LPVOID arg) {
	PWorkerCtx ctx = (PWorkerCtx)arg;
	ctx->partialCount = 0;
	for (int* curr = ctx->start; curr < ctx->end; curr++) {
		ctx->partialCount += *curr;
	}
	return 0;
}

VOID CtxInit(PWorkerCtx pctx, PINT start, PINT end) {
	pctx->start = start;
	pctx->end = end;
	pctx->partialCount = 0;
}

LONG ParallelSum(INT vals[], DWORD size) {
	SYSTEM_INFO si;
	DWORD nParts, partSize;
	DWORD tid;
	HANDLE threads[MAX_PARTS];
	WorkerCtx ctx[MAX_PARTS];

	GetSystemInfo(&si);
	nParts = si.dwNumberOfProcessors;
	partSize = size / nParts;

	for (int i = 0; i < nParts; ++i) {
		int *start = vals + i * partSize;
		int *end = (i < nParts - 1) ? start + partSize :
			vals + size;
		CtxInit(ctx + i, start, end);
		threads[i] = CreateThread(
			NULL,
			0,
			WorkerFunc,
			&ctx,
			0,
			&tid
		);
	}
	return 0;

}
int main()
{
    std::cout << "Hello World!\n"; 
}
