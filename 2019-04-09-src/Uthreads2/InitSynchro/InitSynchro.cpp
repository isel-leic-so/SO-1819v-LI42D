// InitSynchro.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Include/UThread.h"
#include "../Include/USynch.h"

VOID InitNotifierThreadFunc(UT_ARGUMENT arg) {
	printf("InitNotifierThreadFunc begin\n");
	PEVENT evt = (PEVENT)arg;
	printf("Start initialization!\n");
	Sleep(3000);
	printf("End initialization!\n");
	//EventSet(evt);

	printf("InitNotifierThreadFunc end\n");
}
VOID InitWaiterThreadFunc(UT_ARGUMENT arg) {
	EVENT initDone;
	printf("InitWaiterThreadFunc begin\n");
	EventInit(&initDone, FALSE);
	UtCreate(InitNotifierThreadFunc, &initDone);

	UtYield();
	printf("Wait for thread notification!\n");
	EventWait(&initDone);
	printf("InitWaiterThreadFunc done!\n");
	printf("Use state initialized\n");
	UtYield();
	printf("InitWaiterThreadFunc end!");
}

VOID InitSynchroTest() {
	UtCreate(InitWaiterThreadFunc, NULL);
	UtRun();
}


int main()
{
	UtInit();
	InitSynchroTest();
	UtEnd();
    return 0;
}

