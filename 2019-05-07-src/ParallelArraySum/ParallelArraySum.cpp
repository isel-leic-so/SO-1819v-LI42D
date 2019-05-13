// ParallelArraySum.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Chrono.h"

#define MAX_PARTS 64
#define MAX_VALS (1000*100)

INT values[MAX_VALS];

VOID InitValues() {
	for (DWORD i = 0; i < MAX_VALS; ++i) {
		values[i] = 1;
	}

}
typedef struct {
	INT * start;
	INT *end;
	LONG partialCount;
	//BYTE dummy[64];
} WorkerCtx, *PWorkerCtx;

DWORD WINAPI WorkerFunc(LPVOID arg) {
	PWorkerCtx ctx = (PWorkerCtx)arg;
	long partial = 0;
	
	for (int* curr = ctx->start; curr < ctx->end; curr++) {
		partial += *curr;
	}
	ctx->partialCount = partial;
	return 0;
}

VOID CtxInit(PWorkerCtx pctx, PINT start, PINT end) {
	pctx->start = start;
	pctx->end = end;
	pctx->partialCount = 0;
}

typedef LONG(*Adder)(INT vals[], DWORD size);

#define NTRIES 20
LONG TestCountTicks(Adder adder, const char *msg, INT vals[], DWORD size) {
	printf("Test of: %s\n", msg);
	DWORD minTime = MAXDWORD;
	LONG res;

	for (int t = 0; t < NTRIES; ++t) {
		DWORD startTime = GetTickCount();

		res = adder(vals, size);
		DWORD endTime = GetTickCount();
		if (minTime > (endTime - startTime))
			minTime = endTime - startTime;
	}

	printf("Total=%ld, Done in %d ms!\n", res, minTime);
	return res;
}

LONG SequentialSum(INT vals[], DWORD size) {
	LONG res = 0;
	for (DWORD i = 0; i < size; ++i)
		res += vals[i];
	return res;
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

	for (DWORD i = 0; i < nParts; ++i) {
		int *start = vals + i * partSize;
		int *end = (i < nParts - 1) ? start + partSize :
			vals + size;

		CtxInit(ctx + i, start, end);
		threads[i] = CreateThread(
			NULL,
			0,
			WorkerFunc,
			ctx+i,
			0,
			&tid
		);
	}

	LONG total = 0;

	for (int i = 0; i < nParts; ++i) {
		WaitForSingleObject(threads[i], INFINITE);
		//printf("partial add[%d] = %d\n", i, ctx[i].partialCount);

		total += ctx[i].partialCount;
		CloseHandle(threads[i]);
	}
	return total;
}



LONG Test(Adder adder, const char *msg, INT vals[], DWORD size) {
	printf("Test of: %s\n", msg);
	Chronometer chrono;
	__int64 minTime = LLONG_MAX;
	LONG res;

	for (int t = 0; t < NTRIES; ++t) {
		chrono.Start();

		res = adder(vals, size);
		chrono.End();
		if (minTime > chrono.GetMicros())
			minTime = chrono.GetMicros();
	}

	printf("Total=%ld, Done in %I64d micros!\n\n", res, minTime);
	return res;
}


int main()
{

	InitValues();
	Test(SequentialSum, "Soma sequential", values, MAX_VALS);

	Test(ParallelSum, "Soma paralela", values, MAX_VALS);

	return 0;
}
