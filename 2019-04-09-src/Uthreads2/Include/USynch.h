/////////////////////////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread library:
//   User threads supporting cooperative multithreading.
//
// Authors:
//   Carlos Martins, João Trindade, Duarte Nunes, Jorge Martins
// 

#pragma once

#include <Windows.h>

#ifndef USYNCH_DLL
#define USYNCH_API __declspec(dllimport)
#else
#define USYNCH_API __declspec(dllexport)
#endif

typedef struct Event {
	BOOL Signaled;
	LIST_ENTRY Waiters;
} EVENT, *PEVENT;

#ifdef __cplusplus
extern "C" {
#endif

USYNCH_API
VOID EventInit (PEVENT Event, BOOL initialState);

FORCEINLINE
BOOL EventValue (PEVENT Event) {
	return Event->Signaled; 
}

USYNCH_API
VOID EventWait (PEVENT Event);

USYNCH_API
VOID EventSet (PEVENT Event);

/*--------------------------------------------------------

	Mutexes
---------------------------------------------------------*/


//
// A mutex, containing the handle of the user thread that acquired RecursionCounter times the Mutex.
// If Owner is NULL, then the Mutex is free.
//

typedef struct MUTEX {
	LIST_ENTRY WaitListHead;
	ULONG RecursionCounter;
	HANDLE Owner;
} MUTEX, *PMUTEX;

//
// Initializes a mutex instance. If Owned is TRUE, then the current thread becomes the owner.
//

USYNCH_API
VOID MutexInit (PMUTEX Mutex, BOOL Owned);

//
// Acquires the specified mutex, blocking the current thread if the mutex is not free.
//
USYNCH_API
VOID MutexAcquire(PMUTEX Mutex);


//
// Releases the specified mutex, eventually unblocking a waiting thread to which the
// ownership of the mutex is transfered.
//

USYNCH_API
VOID MutexRelease (PMUTEX Mutex);


//
// A semaphore, containing the current number of permits, upper bounded by Limit.
//



typedef struct _SEMAPHORE {
	LIST_ENTRY WaitListHead;
	ULONG Permits;
	ULONG Limit;
} SEMAPHORE, *PSEMAPHORE;



//
// Initializes a semaphore instance. Permits is the starting number of available permits and 
// Limit is the maximum number of permits allowed for the specified semaphore instance.
//
USYNCH_API VOID SemaphoreInit (
	 PSEMAPHORE Semaphore,
	 ULONG Permits,
	 ULONG Limit
);


//
// Gets the specified number of permits from the semaphore. If there aren't enough permits available,  
// the calling thread is blocked until they are added by a call to SemaphoreRelease().
//

USYNCH_API VOID SemaphoreAcquire (
	 PSEMAPHORE Semaphore,
	 ULONG Permits
);

//
// Adds the specified number of permits to the semaphore, eventually unblocking waiting threads.
//

USYNCH_API VOID SemaphoreRelease (
	 PSEMAPHORE Semaphore,
	 ULONG Permits
);

#ifdef __cplusplus
} // extern "C"
#endif
