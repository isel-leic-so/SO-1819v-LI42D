///////////////////////////////////////////////////////////
//
// CCISEL 
// 2007-2010
//
// UThread library:
//     User threads supporting cooperative multithreading.
//     The current version of the library provides:
//        - Threads
//        - Mutexes
//        - Semaphores
//
// Authors: Carlos Martins, João Trindade, Duarte Nunes
// 
// 
 
#include <crtdbg.h>

#include "List.h"
#include "WaitBlock.h"

#include "USynch.h"
#include "Uthread.h"


//#define WITH_SPIN
#define WITH_CRITICAL 1

VOID SPEnter(PSPINLOCK sl) {
	while (TRUE) {
		while (*sl == 1) ;
		LONG old = InterlockedExchange(sl, 1);
		if (old == 0)
			break;
	}
}

VOID SPLeave(PSPINLOCK sl) {
	InterlockedExchange(sl, 0);
}

//
// Initializes a mutex instance. If Owned is TRUE, then the current thread becomes the owner.
//

VOID MutexInit(PMUTEX Mutex, BOOL Owned)
{
#ifdef WITH_SPIN
	Mutex->Lock = 0;
#elif WITH_CRITICAL
	InitializeCriticalSection(&Mutex->Lock);
#endif
	InitializeListHead(&Mutex->WaitListHead);
	Mutex->Owner = Owned ? UtSelf() : NULL;
	Mutex->RecursionCounter = Owned ? 1 : 0;
	
}

//
// Acquires the specified mutex, blocking the current thread if the mutex is not free.
//

VOID MutexAcquire(PMUTEX Mutex) {
	WAIT_BLOCK WaitBlock;
 
	if (Mutex->Owner == UtSelf()) {
		//
		// Recursive aquisition. Increment the recursion counter.
		//
		Mutex->RecursionCounter += 1;
		return;
	} 
	
#ifdef WITH_SPIN
	SPEnter(&Mutex->Lock);
#elif WITH_CRITICAL
	EnterCriticalSection(&Mutex->Lock);
#endif
	if (Mutex->Owner == NULL) {

		//
		// Mutex is free. Acquire the mutex by setting its owner to the current thread.
		//

		Mutex->Owner = UtSelf();
		Mutex->RecursionCounter = 1;
		
#ifdef WITH_SPIN
		SPLeave(&Mutex->Lock);
#elif WITH_CRITICAL
		LeaveCriticalSection(&Mutex->Lock);
#endif
		return;
	}  

	//
	// Insert the running thread in the wait list.
	//

	InitializeWaitBlock(&WaitBlock);
	InsertTailList(&Mutex->WaitListHead, &WaitBlock.Link);
	 
#ifdef WITH_SPIN
	SPLeave(&Mutex->Lock);
#elif WITH_CRITICAL
	LeaveCriticalSection(&Mutex->Lock);
#endif
	//
	// Schedule another thread. When the thread execute again, it will have ownership of the mutex.
	//
		
	UtDeactivate();

	_ASSERTE(Mutex->Owner == UtSelf());
}

//
// Releases the specified mutex, eventually unblocking a waiting thread to which the
// ownership of the mutex is transfered.
//

VOID MutexRelease(PMUTEX Mutex) {
	PWAIT_BLOCK WaitBlock;

	
	_ASSERTE(Mutex->Owner == UtSelf());

	if ((Mutex->RecursionCounter -= 1) > 0) {
		
		//
		// The current thread is still the owner of the mutex.
		//

		return;
	}
	
#ifdef WITH_SPIN
	SPEnter(&Mutex->Lock);
#elif WITH_CRITICAL
	EnterCriticalSection(&Mutex->Lock);
#endif
	if (IsListEmpty(&Mutex->WaitListHead)) {

		//
		// No threads are blocked; the mutex becomes free.
		//

		Mutex->Owner = NULL;
		//LeaveCriticalSection(&Mutex->Lock);
#ifdef WITH_SPIN
		SPLeave(&Mutex->Lock);
#elif WITH_CRITICAL
		LeaveCriticalSection(&Mutex->Lock);
#endif
		return;
	}

	//
	// Get the next blocked thread and transfer mutex ownership to it.
	//

	WaitBlock = CONTAINING_RECORD(RemoveHeadList(&Mutex->WaitListHead), WAIT_BLOCK, Link);
	Mutex->Owner = WaitBlock->Thread;
	Mutex->RecursionCounter = 1;
#ifdef WITH_SPIN
	SPLeave(&Mutex->Lock);
#elif WITH_CRITICAL
	LeaveCriticalSection(&Mutex->Lock);
#endif
	//LeaveCriticalSection(&Mutex->Lock);
	//
	// Put the thread in the ready list.
	//
	
	UtActivate(WaitBlock->Thread);
	 
}
