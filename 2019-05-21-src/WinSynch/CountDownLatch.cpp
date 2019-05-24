// CountDownLatch.cpp : Defines the entry point for the console application.
//

/*----------------------------------------------------------------------
 Synchronizer for workers completion synchronization
 -----------------------------------------------------------------------*/
#include "stdafx.h"


VOID CDL_Init (PCOUNTDOWN_LATCH latch, ULONG value) {
	latch->signaled = CreateEvent(NULL, TRUE, FALSE, NULL);
	latch->counter = value; 
	latch->initialCount = value;
}


// Normally the count down latch has only one using, so
// this function doesn´t exist in most CountDownLatch implementations
VOID CDL_Reset(PCOUNTDOWN_LATCH latch) {
	latch->counter = latch->initialCount;
	ResetEvent(latch->signaled);
}

VOID  CDL_Wait (PCOUNTDOWN_LATCH latch) { 

	if (latch->counter <= 0)
		return;
	 
	WaitForSingleObject(latch->signaled, INFINITE); 
}

VOID CDL_Signal (PCOUNTDOWN_LATCH latch) {
	int res;
 
	res = InterlockedDecrement(&latch->counter);
	
	if (res == 0)
		SetEvent(latch->signaled);
}


VOID CDL_Destroy(PCOUNTDOWN_LATCH latch) {
	CloseHandle(latch->signaled);
}

 