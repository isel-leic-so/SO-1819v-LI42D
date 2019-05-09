#pragma once

typedef struct CDL {
	HANDLE Signaled;
#ifdef WITH_MUTEX
	HANDLE Mutex;
#endif
	DWORD Counter; //InitialCounter;
} CDL, *PCDL;

VOID CDL_Init(PCDL latch, DWORD value);
	 

VOID CDL_Wait(PCDL latch);
	 

VOID CDL_Signal(PCDL latch);

VOID CDL_Destroy(PCDL latch);
	 
