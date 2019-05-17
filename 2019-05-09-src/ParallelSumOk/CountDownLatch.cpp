#include "stdafx.h"

#include "CountDownLatch.h"

VOID CDL_Init(PCDL cdl, DWORD initialCounter) {
	cdl->Counter = initialCounter;
	cdl->WorkDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	//cdl->Mutex = CreateMutex(NULL, FALSE, NULL);
}

VOID CDL_Wait(PCDL cdl) {
	if (cdl->Counter > 0)
		WaitForSingleObject(cdl->WorkDone, INFINITE);
}

VOID CDL_Signal(PCDL cdl) {
	DWORD localCounter;
	/*WaitForSingleObject(cdl->Mutex, INFINITE);
	localCounter = --cdl->Counter;
	ReleaseMutex(cdl->Mutex);*/

	localCounter = InterlockedDecrement(&cdl->Counter);

	if (localCounter == 0)
		SetEvent(cdl->WorkDone);
}

VOID CDL_Destroy(PCDL cdl) {
	//CloseHandle(cdl->Mutex);
	CloseHandle(cdl->WorkDone);
}
