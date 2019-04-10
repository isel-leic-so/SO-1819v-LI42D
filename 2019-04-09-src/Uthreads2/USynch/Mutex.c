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
 
//
// Initializes a mutex instance. If Owned is TRUE, then the current thread becomes the owner.
//

VOID MutexInit(PMUTEX Mutex, BOOL Owned)
{
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
		// Recursive aquisition. Increment the recursion counter.
		Mutex->RecursionCounter += 1;
	} else {
		 
		if (Mutex->Owner == NULL) {
			// Mutex is free. Acquire the mutex by setting its owner to the current thread.
			Mutex->Owner = UtSelf();
			Mutex->RecursionCounter = 1;
		}
		else {
			// Insert the running thread in the wait list.
			InitializeWaitBlock(&WaitBlock);
			InsertTailList(&Mutex->WaitListHead, &WaitBlock.Link);
			// Schedule another thread. When the thread execute again, it will have ownership of the mutex.
			UtDeactivate();
			_ASSERTE(Mutex->Owner == UtSelf());
		}
		 
	}
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

	
	if (IsListEmpty(&Mutex->WaitListHead)) {

		//
		// No threads are blocked; the mutex becomes free.
		//

		Mutex->Owner = NULL;
	
		return;
	}

	//
	// Get the next blocked thread and transfer mutex ownership to it.
	//
	 
	WaitBlock = CONTAINING_RECORD(RemoveHeadList(&Mutex->WaitListHead), 
		WAIT_BLOCK, Link);
	 
	Mutex->Owner = WaitBlock->Thread;
	Mutex->RecursionCounter = 1;
		
	//
	// Put the thread in the ready list.
	//

	UtActivate(WaitBlock->Thread);
	 
}
