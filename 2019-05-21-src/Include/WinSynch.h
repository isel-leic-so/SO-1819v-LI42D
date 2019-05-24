#pragma once

#ifdef WINSYNCH_EXPORTS
#define WINSYNCH_API_ENTRY __declspec(dllexport)
#else
#define WINSYNCH_API_ENTRY __declspec(dllimport)
#endif



#ifdef __cplusplus 
extern "C" {
#endif

// CoundownLatch
typedef struct COUNTDOWN_LATCH {
	HANDLE signaled;
	ULONG counter;
	ULONG initialCount;
} COUNTDOWN_LATCH, *PCOUNTDOWN_LATCH;
 

WINSYNCH_API_ENTRY
VOID CDL_Init(PCOUNTDOWN_LATCH latch, ULONG value);
		
WINSYNCH_API_ENTRY
VOID CDL_Wait(PCOUNTDOWN_LATCH latch);
	
WINSYNCH_API_ENTRY
VOID CDL_Signal(PCOUNTDOWN_LATCH latch);
	
WINSYNCH_API_ENTRY
VOID CDL_Reset(PCOUNTDOWN_LATCH latch);

WINSYNCH_API_ENTRY
VOID CDL_Destroy(PCOUNTDOWN_LATCH latch);

//
// Generic Simple Queue
//
typedef struct SQUEUE {
	LIST_ENTRY list;
	INT nItems, maxItems;
	HANDLE itemsAvaiable, spaceAvaiable;
	BOOL terminated;
	HANDLE closingQueue;
	CRITICAL_SECTION lock;
	
} SQUEUE, *PSQUEUE;


WINSYNCH_API_ENTRY
VOID SQueueInit(PSQUEUE queue, DWORD maxItems);


/* bloqueia a thread invocante enquanto a fila está vazia */
WINSYNCH_API_ENTRY
PLIST_ENTRY SQueueGet(PSQUEUE queue);

/* coloca um novo item na fila */
WINSYNCH_API_ENTRY
BOOL SQueuePut(PSQUEUE queue, PLIST_ENTRY  data);

WINSYNCH_API_ENTRY
BOOL SQueueClose(PSQUEUE queue);

WINSYNCH_API_ENTRY
VOID SQueueDestroy(PSQUEUE queue);

//
// Generic Queue
//
typedef struct QUEUE {
	LIST_ENTRY list;
	INT nItems;
	HANDLE itemsAvaiable;
	HANDLE mutex;
	HANDLE terminateQueue;
	BOOL terminated;
} QUEUE, *PQUEUE;

WINSYNCH_API_ENTRY
VOID QueueInit(PQUEUE queue);

WINSYNCH_API_ENTRY
VOID QueueClose(PQUEUE queue);

WINSYNCH_API_ENTRY
BOOL QueueDestroy(PQUEUE queue);

/* bloqueia a thread invocante enquanto a fila está vazia */
WINSYNCH_API_ENTRY
LPVOID QueueGet(PQUEUE queue);

/* coloca um novo item na fila */
WINSYNCH_API_ENTRY
BOOL QueuePut(PQUEUE queue, LPVOID  data);


// User Mutex

typedef struct UserMutex {
	DWORD Owner;
	DWORD RecursionCount;
	volatile LONG Count;
	HANDLE Wakeup;
	DWORD SpinCount;
} USERMUTEX, *PUSERMUTEX;


WINSYNCH_API_ENTRY
VOID UMtxInit(PUSERMUTEX um);
		 
WINSYNCH_API_ENTRY
VOID UMtxAcquire(PUSERMUTEX um);

WINSYNCH_API_ENTRY
VOID UMtxRelease(PUSERMUTEX um);
	
WINSYNCH_API_ENTRY
VOID UMtxDestroy(PUSERMUTEX um);
		 
// Cyclic Barrier

typedef struct CyclicBarrier {
	LONG total;
	HANDLE *barrierReached;
	LONG count;
	DWORD tlsSlot;
	LONG usedEvents;
} CYCLIC_BARRIER, *PCYCLIC_BARRIER;

WINSYNCH_API_ENTRY
VOID CB_Init(PCYCLIC_BARRIER b, LONG count);
		 
WINSYNCH_API_ENTRY
VOID CB_Wait(PCYCLIC_BARRIER b);
		 
WINSYNCH_API_ENTRY
VOID CB_Destroy(PCYCLIC_BARRIER b);
		 
#ifdef __cplusplus 
}
#endif