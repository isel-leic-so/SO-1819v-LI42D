#pragma once

#ifdef THREADPOOL0_EXPORTS
#define THREADPOOL_API_ENTRY __declspec(dllexport)
#else
#define THREADPOOL_API_ENTRY __declspec(dllimport)
#endif

typedef VOID(*WORK_FUNC)(LPVOID arg);

typedef struct _threadPool {
	HANDLE *workerThreads;
	// request items queue
	SQUEUE workItems;
	LONG destroyed;
	 
} THREAD_POOL, *PTHREAD_POOL;

#ifdef __cplusplus 



extern "C" {
#endif



THREADPOOL_API_ENTRY
VOID TPInit(PTHREAD_POOL tp);

THREADPOOL_API_ENTRY
VOID TPDestroy(PTHREAD_POOL tp);

THREADPOOL_API_ENTRY
VOID TPQueueWorkItem(PTHREAD_POOL tp, WORK_FUNC func, LPVOID arg);

THREADPOOL_API_ENTRY
VOID MyQueueWorkItem( WORK_FUNC func, LPVOID arg);

THREADPOOL_API_ENTRY
VOID MyTPInit();

THREADPOOL_API_ENTRY
VOID MyTPDestroy();





#ifdef __cplusplus 
#define WITH_CLIENT_SYNCHRO
#ifdef WITH_CLIENT_SYNCHRO

#ifndef THREADPOOL0_EXPORTS
class TPWrapper {
private:
	static LONG initialized, terminated;
	
public:
	TPWrapper() {
		if (InterlockedExchange(&initialized, 1) == 0)
			MyTPInit();
	}

	~TPWrapper() {
		if (InterlockedExchange(&terminated, 1) == 0)
			MyTPDestroy();
	}
};


static TPWrapper tp;

#endif
#endif
}
#endif