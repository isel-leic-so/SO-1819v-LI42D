#include "stdafx.h"
#include "cdl.h"

//
// Initializes a Countdownlatch instance.  
//

VOID
CDL_Init(PCDL latch, DWORD value) {
	latch->Signaled = CreateEvent(NULL, TRUE, FALSE, NULL);
#ifdef WITH_MUTEX
	latch->Mutex = CreateMutex(NULL, FALSE, NULL);
#endif
	latch->Counter = value;
}

VOID CDL_Wait(PCDL latch) {
	WaitForSingleObject(latch->Signaled, INFINITE);	 
}

VOID CDL_Signal(PCDL latch) {
#ifdef WITH_MUTEX
	WaitForSingleObject(latch->Mutex, INFINITE);
	int res = --latch->Counter;
	ReleaseMutex(latch->Mutex);
#else
	int res = InterlockedDecrement(&latch->Counter);
#endif
	if (res == 0)
		SetEvent(latch->Signaled);	 
}

VOID CDL_Destroy(PCDL latch) {
#ifdef WITH_MUTEX
	CloseHandle(latch->Mutex);
#endif
	CloseHandle(latch->Signaled);
}