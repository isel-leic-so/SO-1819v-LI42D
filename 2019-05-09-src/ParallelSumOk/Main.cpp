// ParallelizationExperience.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Chrono.h"
#include "CountDownLatch.h"
#include "../../2019-05-16-src/Include/Threadpool.h"

#define NVALS (100*1000)
#define MAX_PARTS 64

DWORD values[NVALS];

typedef struct {
	DWORD *start, *end;
	LONG partialResult;

	HANDLE partialWorkDone;

	PCDL cdl;
	// the next field is used tio guarantee the the line caches don't share
	// contexts, in order to avoid the false sharing problem.
	// Note that in this case a better way to avoid false sharing is to use a local accumulator in worker threads
	// as we done now.
	char dummy[64];
} WORKER_CTX, *PWORKER_CTX;

VOID InitValues() {
	for (int i = 0; i < NVALS; ++i) {
		values[i] = 1;
	}
}

DWORD WINAPI WorkerFunc(LPVOID arg) {
	PWORKER_CTX ctx = (PWORKER_CTX)arg;
	long partial = 0;

	 
	for (DWORD *pi = ctx->start; pi < ctx->end; ++pi) {
		partial += *pi;
	}
	ctx->partialResult = partial;
	return 0;
}

DWORD WINAPI WorkerFuncTP(LPVOID arg) {
	PWORKER_CTX ctx = (PWORKER_CTX)arg;
	long partial = 0;


	for (DWORD *pi = ctx->start; pi < ctx->end; ++pi) {
		partial += *pi;
	}
	ctx->partialResult = partial;
	SetEvent(ctx->partialWorkDone);
	return 0;
}



LONG SequentialSum(DWORD vals[], DWORD size) {
	LONG res = 0;
	for (DWORD i = 0; i < size; ++i)
		res += vals[i];
	return res;
}

static VOID CtxInit(PWORKER_CTX pctx, PDWORD start, PDWORD end) {
	pctx->start = start;
	pctx->end = end;
	pctx->partialResult = 0;

}

LONG ParallelSum(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	HANDLE threadHandles[MAX_PARTS];
	WORKER_CTX ctx[MAX_PARTS];

	GetSystemInfo(&si);
	int nParts = si.dwNumberOfProcessors;

	int partSize = size / nParts;

	for (int i = 0; i < nParts; ++i) {
		PDWORD start = ctx[i].start = vals + i*partSize;
		PDWORD end = (i < nParts -1) ?
			ctx[i].start + partSize : end = vals + size;

		CtxInit(ctx + i, start, end);
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
	for (int i = 0; i < nParts; ++i) {
		WaitForSingleObject(threadHandles[i], INFINITE);
		total += ctx[i].partialResult;
		CloseHandle(threadHandles[i]);
	}
	return total;
}

static HANDLE CtxInitTp(PWORKER_CTX pctx, PDWORD start, PDWORD end) {
	pctx->start = start;
	pctx->end = end;
	pctx->partialResult = 0;
	pctx->partialWorkDone = CreateEvent(NULL,
		TRUE /*manual */,
		FALSE,
		NULL);
	return pctx->partialWorkDone;
}

LONG ParallelSumTP(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	DWORD nParts, partSize;

	WORKER_CTX ctx[MAX_PARTS];

	GetSystemInfo(&si);
	nParts = si.dwNumberOfProcessors;
	partSize = size / nParts;

	for (DWORD i = 0; i < nParts; ++i) {
		DWORD *start = vals + i * partSize;
		DWORD *end = (i < nParts - 1) ? start + partSize :
			vals + size;

		CtxInitTp(ctx + i, start, end);
		QueueUserWorkItem(WorkerFuncTP, ctx + i, 0);
	}

	LONG total = 0;

	for (DWORD i = 0; i < nParts; ++i) {
		// And now, I can we synchronize with the completion of partial adders?
		WaitForSingleObject(ctx[i].partialWorkDone, INFINITE);
		// WaitForSingleObject(threads[i], INFINITE);
		
		//printf("partial add[%d] = %d\n", i, ctx[i].partialCount);

		total += ctx[i].partialResult;
		//CloseHandle(threads[i]);
		CloseHandle(ctx[i].partialWorkDone);
		 
	}
	return total;
}

LONG ParallelSumTPMW(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	DWORD nParts, partSize;

	WORKER_CTX ctx[MAX_PARTS];
	HANDLE workEvents[MAX_PARTS];

	GetSystemInfo(&si);
	nParts = si.dwNumberOfProcessors;
	partSize = size / nParts;

	for (DWORD i = 0; i < nParts; ++i) {
		DWORD *start = vals + i * partSize;
		DWORD *end = (i < nParts - 1) ? start + partSize :
			vals + size;

		workEvents[i] = CtxInitTp(ctx + i, start, end);
		QueueUserWorkItem(WorkerFuncTP, ctx + i, 0);
	}

	LONG total = 0;

	WaitForMultipleObjects(nParts, workEvents, TRUE, INFINITE);

	for (DWORD i = 0; i < nParts; ++i) {
		 
		total += ctx[i].partialResult;
		//CloseHandle(threads[i]);
		CloseHandle(ctx[i].partialWorkDone);

	}
	return total;
}


static VOID CtxInitTpCdl(
	PWORKER_CTX pctx, PDWORD start, PDWORD end, PCDL cdl) {
	pctx->start = start;
	pctx->end = end;
	pctx->partialResult = 0;
	pctx->cdl = cdl;
}

DWORD WINAPI WorkerFuncTPCDL(LPVOID arg) {
	PWORKER_CTX ctx = (PWORKER_CTX)arg;
	long partial = 0;


	for (DWORD *pi = ctx->start; pi < ctx->end; ++pi) {
		partial += *pi;
	}
	ctx->partialResult = partial;
	CDL_Signal(ctx->cdl);
	return 0;
}

LONG ParallelSumTPCDL(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	DWORD nParts, partSize;

	WORKER_CTX ctx[MAX_PARTS];
	CDL cdl;

	GetSystemInfo(&si);
	nParts = si.dwNumberOfProcessors;
	partSize = size / nParts;

	CDL_Init(&cdl, nParts);

	for (DWORD i = 0; i < nParts; ++i) {
		DWORD *start = vals + i * partSize;
		DWORD *end = (i < nParts - 1) ? start + partSize :
			vals + size;

		CtxInitTpCdl(ctx + i, start, end, &cdl);
		QueueUserWorkItem(WorkerFuncTPCDL, ctx + i, 0);
	}

	LONG total = 0;

	CDL_Wait(&cdl);

	for (DWORD i = 0; i < nParts; ++i) 
		total += ctx[i].partialResult;
	
	CDL_Destroy(&cdl);
	return total;
}

LONG ParallelSumSOTPCDL(DWORD vals[], DWORD size) {
	SYSTEM_INFO si;
	DWORD nParts, partSize;

	WORKER_CTX ctx[MAX_PARTS];
	CDL cdl;

	GetSystemInfo(&si);
	nParts = si.dwNumberOfProcessors;
	partSize = size / nParts;

	CDL_Init(&cdl, nParts);

	for (DWORD i = 0; i < nParts; ++i) {
		DWORD *start = vals + i * partSize;
		DWORD *end = (i < nParts - 1) ? start + partSize :
			vals + size;

		CtxInitTpCdl(ctx + i, start, end, &cdl);
		TpQueueItem(WorkerFuncTPCDL, ctx + i);
	}

	LONG total = 0;

	CDL_Wait(&cdl);

	for (DWORD i = 0; i < nParts; ++i)
		total += ctx[i].partialResult;

	CDL_Destroy(&cdl);
	return total;
}

// type representing an array sum function
typedef LONG(*Adder)(DWORD vals[], DWORD size);


#define NTRIES 10
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

	TpInit();

	Test(SequentialSum, "Sequential", values, NVALS);
	Test(ParallelSum, "Parallel", values, NVALS);
	Test(ParallelSumTP, "Parallel Thread Pool", values, NVALS);
	Test(ParallelSumTPMW, 
		"Parallel Thread Pool With Multiple Wait", 
		values, NVALS);
	Test(ParallelSumTPCDL,
		"Parallel Thread Pool With CountDownLatch Synchro", 
		values, NVALS);
	Test(ParallelSumSOTPCDL,
		"SO Parallel Thread Pool With CountDownLatch Synchro",
		values, NVALS);

    return 0;
}

