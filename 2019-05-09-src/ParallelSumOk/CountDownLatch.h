#pragma once

typedef struct {
	DWORD Counter;
	HANDLE WorkDone;
	//HANDLE Mutex;

} CDL, *PCDL;

VOID CDL_Init(PCDL cdl, DWORD initialCounter);

VOID CDL_Wait(PCDL cdl);

VOID CDL_Signal(PCDL cdl);

VOID CDL_Destroy(PCDL cdl);

