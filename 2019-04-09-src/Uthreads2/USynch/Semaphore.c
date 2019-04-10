
#include "WaitBlock.h"
#include "USynch.h"

//
// Wait block used to queue requests on semaphores.
//
typedef struct _SEMAPHORE_WAIT_BLOCK {
	WAIT_BLOCK Header;
	ULONG RequestedPermits;
} SEMAPHORE_WAIT_BLOCK, *PSEMAPHORE_WAIT_BLOCK;


//
// Initializes the specified semaphore wait block.
//

FORCEINLINE
VOID
InitializeSemaphoreWaitBlock(
PSEMAPHORE_WAIT_BLOCK SemaphoreWaitBlock,
ULONG RequestedPermits
)
{
	InitializeWaitBlock(&SemaphoreWaitBlock->Header);
	SemaphoreWaitBlock->RequestedPermits = RequestedPermits;
}




//
// Initializes a semaphore instance. Permits is the starting number of available permits and 
// Limit is the maximum number of permits allowed for the specified semaphore instance.
//

VOID SemaphoreInit (PSEMAPHORE Semaphore, ULONG Permits, ULONG Limit) {
	InitializeListHead(&Semaphore->WaitListHead);
	Semaphore->Permits = Permits;
	Semaphore->Limit = Limit;
}

//
// Gets the specified number of permits from the semaphore. If there aren't enough permits available,  
// the calling thread is blocked until they are added by a call to SemaphoreRelease().
//

VOID
SemaphoreAcquire (PSEMAPHORE Semaphore, ULONG Permits) {
	SEMAPHORE_WAIT_BLOCK WaitBlock;

	//
	// If there are enough permits available, get them and keep running.
	//
	if (Semaphore->Permits >= Permits) {
		Semaphore->Permits -= Permits;
		return;
	}

	//
	// There are no permits available. Insert the running thread in the wait list.
	//

	InitializeSemaphoreWaitBlock(&WaitBlock, Permits);   
	InsertTailList(&Semaphore->WaitListHead, &WaitBlock.Header.Link);

	//
	// remove the current thread from the ready list.
	//
	UtDeactivate();
}

//
// Adds the specified number of permits to the semaphore, eventually unblocking waiting threads.
//

VOID SemaphoreRelease (PSEMAPHORE Semaphore, ULONG Permits) {
	PLIST_ENTRY ListHead;
	PSEMAPHORE_WAIT_BLOCK WaitBlock;
	PLIST_ENTRY WaitEntry;


	if ((Semaphore->Permits += Permits) > Semaphore->Limit) {
		Semaphore->Permits = Semaphore->Limit;
	}

	ListHead = &Semaphore->WaitListHead;

	//
	// Release all blocked thread whose request can be satisfied.
	//
	while (Semaphore->Permits > 0 && (WaitEntry = ListHead->Flink) != ListHead) {
		WaitBlock = CONTAINING_RECORD(WaitEntry, SEMAPHORE_WAIT_BLOCK, Header.Link);

		if (Semaphore->Permits < WaitBlock->RequestedPermits) {
			
			//
			// We stop at the first request that cannot be satisfied to ensure FIFO ordering.
			//

			break;
		}

		Semaphore->Permits -= WaitBlock->RequestedPermits;
		RemoveHeadList(ListHead);
		UtActivate(WaitBlock->Header.Thread);
	}
}
