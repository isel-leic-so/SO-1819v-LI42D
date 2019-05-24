// UserMutex.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"


#include <crtdbg.h>

#define WITH_SPINLOCK

// tells if some initial spinning is using trying to acquire the mutex
//#define WITH_SPINLOCK


VOID UMtxInit(PUSERMUTEX um) {
	um->Owner = 0;
	um->RecursionCount = 0;
	um->Count = 0;
#ifdef WITH_SPINLOCK
	um->SpinCount = 3000;
#endif
	um->Wakeup = CreateEvent(NULL, FALSE, FALSE, NULL);
}

VOID UMtxAcquire(PUSERMUTEX um) {
	DWORD res=1;
	if (um->Owner == GetCurrentThreadId())
		// current thread already owns the mutex
		um->RecursionCount++;
	else  {
#ifdef WITH_SPINLOCK
		DWORD times= um->SpinCount; 
		 
		for (DWORD i = 0; i < times; ++i) {
			if (um->Count == 0 && 
				(res = InterlockedCompareExchange(&um->Count, 1, 0)) == 0)
				break;	
		}
				
#endif
		// if not successfull spin mutex acquisition, 
		// a last try before passive wait on event
		if (res != 0) {
			if ((InterlockedIncrement(&um->Count)) > 1) {
				WaitForSingleObject(um->Wakeup, INFINITE);
			}
		}
	
        um->Owner = GetCurrentThreadId();
	    um->RecursionCount = 1;
	}
}

VOID UMtxRelease(PUSERMUTEX um) {
	 
	_ASSERTE(um->Owner == GetCurrentThreadId());
	if (--um->RecursionCount == 0) {
		um->Owner= NULL;
		// Count value tells how much waiters are
		// if there are waiters signal the event in order to make 
		// a waiter the new owner
		if ((_InterlockedDecrement(&um->Count)) > 0) {
			SetEvent(um->Wakeup);
		}
	}
  
}

VOID UMtxDestroy(PUSERMUTEX um) {
	CloseHandle(um->Wakeup);
}
