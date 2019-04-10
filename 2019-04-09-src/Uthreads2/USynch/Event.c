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

#include "WaitBlock.h"
#include "USynch.h"
#include "UThread.h"


VOID EventInit (PEVENT Event, BOOL initialState) {
	Event->Signaled = initialState;
	InitializeListHead(&Event->Waiters);
}

VOID EventWait (PEVENT Event) {
	if (Event->Signaled == TRUE) {
		Event->Signaled = FALSE;
	} else {
		WAIT_BLOCK WaitBlock;
		WaitBlock.Thread = UtSelf();
		InsertTailList(&Event->Waiters, &WaitBlock.Link);
		UtDeactivate();
	}
}

VOID EventSet (PEVENT Event) {
	if (IsListEmpty(&Event->Waiters)) {
		Event->Signaled = TRUE;
	} else {
		PWAIT_BLOCK WaitBlockPtr =
			CONTAINING_RECORD(RemoveHeadList(&Event->Waiters), WAIT_BLOCK, Link);
		UtActivate(WaitBlockPtr->Thread);
	}
}
