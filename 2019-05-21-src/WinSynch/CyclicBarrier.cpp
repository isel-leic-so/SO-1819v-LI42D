#include "stdafx.h"

/*--------------------------------------------------------------------
  
  Cyclic barrier. This synchronizer isi used for peers rendez-vous. 
  Peers do the rendez-vous by calling barrier "wait" operation. 
  Peers block until all have called "wait". 
  
  This  implementation use one auto-reset event per peer.
  Associated event are stored in peers TLS for easy access.
  It is assumed that the same set of peer threads use the barrier in all 
  synchronization rounds.

  JM, 2015
  --------------------------------------------------------------------*/

VOID CB_Init(PCYCLIC_BARRIER b, LONG count) {
	// Allocate a TLS slot
	b->tlsSlot = TlsAlloc();
	b->total = count;
	b->count = count;
	b->usedEvents = -1;
	b->barrierReached = (PHANDLE) malloc(sizeof(HANDLE)*count);
	for (LONG i = 0; i < count; ++i)
		b->barrierReached[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
}


// get the event for current peer thread
FORCEINLINE static HANDLE GetThreadEvent(PCYCLIC_BARRIER b)
{
	HANDLE currEvent = TlsGetValue(b->tlsSlot);
	if (currEvent == NULL)
	{
		int idx = InterlockedIncrement(&b->usedEvents);
		_ASSERT(idx < b->total);
		currEvent = b->barrierReached[idx];
		TlsSetValue(b->tlsSlot, currEvent);
	}
	return currEvent;

}


// do the barrier wait
FORCEINLINE static VOID CB_WaitOnThreadEvent(PCYCLIC_BARRIER b)
{
	WaitForSingleObject(GetThreadEvent(b), INFINITE);
}


// called by all peers.
// the lastcalling  wake-up the remaining peers (participants).
VOID CB_Wait(PCYCLIC_BARRIER b) {
	int res = InterlockedDecrement(&b->count);
	if (res == 0)  { // round completes, restart and notify
		b->count = b->total;
		for (LONG i = 0; i < b->total; ++i)  {
			// of course, not sinalize the sinalizer peer
			if (b->barrierReached[i] != GetThreadEvent(b))
				SetEvent(b->barrierReached[i]);
		}
	}
	else { // wait on specific event
		CB_WaitOnThreadEvent(b);
	}
}


VOID CB_Destroy(PCYCLIC_BARRIER b) {
	TlsFree(b->tlsSlot);
	for (LONG i = 0; i < b->total; ++i)
		CloseHandle(b->barrierReached[i]);
	free(b->barrierReached);
}