// ParallelizationExperience.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Chrono.h"
#include "cdl.h"

#define NVALS (1000*1000*100)
#define MAX_CORES 64

DWORD values[NVALS];

typedef struct {
	DWORD *start, *end;
	LONG partialResult;
	HANDLE partialWorkDone;
	PCDL pcdl;
	char dummy[64];
} WORKER_CTX, *PWORKER_CTX;

VOID InitValues() {
	for (int i = 0; i < NVALS; ++i) {
		values[i] = 1;
	}
}

DWORD WINAPI WorkerFunc(LPVOID arg) {
	PWORKER_CTX ctx = (PWORKER_CTX)arg;

	ctx->partialResult = 0;
	for (DWORD *pi = ctx->start; pi < ctx->end; ++pi) {
		ctx->partialResult += *pi;
	}

	SetEvent(ctx->partialWorkDone);
	return 0;
}

DWORD WINAPI WorkerFuncCDL(LPVOID arg) {
	PWORKER_CTX ctx = (PWORKER_CTX)arg;

	ctx->partialResult = 0;
	for (DWORD *curr = ctx->start; curr < ctx->end; ++curr) {
		ctx->partialResult += *curr;
	}

	CDL_Signal(ctx->pcdl);
	return 0;
}


LONG ParallelAdderTP(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;	 
	WORKER_CTX ctx[MAX_CORES];
	GetSystemInfo(&si);

	int nCores = si.dwNumberOfProcessors;
	int partSize = size / nCores;
	
	for (int i = 0; i < nCores; ++i) {
		ctx[i].start = vals + i*partSize;
		ctx[i].end = ctx[i].start + partSize;
		if (ctx[i].end > vals + size) ctx[i].end = vals + size;
		ctx[i].partialWorkDone = CreateEvent(NULL, FALSE, FALSE, NULL);
		QueueUserWorkItem(WorkerFunc, ctx + i, 0);
	}
	LONG total = 0;
	for (int i = 0; i < nCores; ++i) {
		WaitForSingleObject(ctx[i].partialWorkDone, INFINITE);
		total += ctx[i].partialResult;
		CloseHandle(ctx[i].partialWorkDone);
	}
	return total;
}

LONG ParallelAdderTPCDL(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	WORKER_CTX ctx[MAX_CORES];
	GetSystemInfo(&si);

	int nParts = si.dwNumberOfProcessors;
	int partSize = size / nParts;
	CDL cdl;
	CDL_Init(&cdl, nParts);
	for (int i = 0; i < nParts; ++i) {
		ctx[i].start = vals + i * partSize;
		ctx[i].end = ctx[i].start + partSize;
		ctx[i].pcdl = &cdl;
		if (ctx[i].end > vals + size) ctx[i].end = vals + size;
	
		QueueUserWorkItem(WorkerFuncCDL, ctx + i, 0);
	}
	LONG total = 0;

	CDL_Wait(&cdl);
	CDL_Destroy(&cdl);
	for (int i = 0; i < nParts; ++i) {
		total += ctx[i].partialResult;
	}
	
	return total;
}


LONG SequentialAdder(DWORD vals[], DWORD size) {
	LONG res = 0;
	for (int i = 0; i < size; ++i)
		res += vals[i];
	return res;
}

LONG ParallelAdder(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	HANDLE threadHandles[MAX_CORES];
	WORKER_CTX ctx[MAX_CORES];

	GetSystemInfo(&si);
	int nCores = si.dwNumberOfProcessors;

	int partSize = size / nCores;

	for (int i = 0; i < nCores; ++i) {
		ctx[i].start = vals + i*partSize;
		ctx[i].end = ctx[i].start + partSize;
		if (ctx[i].end > vals + size) ctx[i].end = vals + size;

		threadHandles[i] = CreateThread(
			NULL,
			0,
			WorkerFunc,
			ctx + i,
			0,
			NULL
			);
	}

	LONG total = 0;
	for (int i = 0; i < nCores; ++i) {
		WaitForSingleObject(threadHandles[i], INFINITE);
		total += ctx[i].partialResult;
		CloseHandle(threadHandles[i]);
	}
	return total;
}

typedef LONG(*Adder)(DWORD vals[], DWORD size);

#define NTRIES 20
LONG TestCountTicks(Adder adder, char *msg, DWORD vals[], DWORD size) {
	printf("Test of: %s\n", msg);
	DWORD minTime = MAXDWORD;
	LONG res;

	for (int t = 0; t < NTRIES; ++t) {
		DWORD startTime = GetTickCount();

		res = adder(vals, size);
		DWORD endTime = GetTickCount();
		if (minTime > (endTime-startTime))
			minTime = endTime - startTime;
	}
 
	printf("Total=%ld, Done in %d ms!\n", res, minTime);
	return res;
}


LONG Test(Adder adder, char *msg, DWORD vals[], DWORD size) {
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

	Test(SequentialAdder, "Sequential", values, NVALS);
	Test(ParallelAdder, "Parallel", values, NVALS);
	Test(ParallelAdderTP, "Parallel Thread Pool", values, NVALS);
	Test(ParallelAdderTPCDL, "Parallel Thread Pool With CDL synch", values, NVALS);

    return 0;
}

