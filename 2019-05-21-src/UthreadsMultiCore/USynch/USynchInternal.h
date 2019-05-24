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

#pragma once

#include <Windows.h>

/*--------------------------------------------------------
Events
---------------------------------------------------------*/

typedef struct Event {
	BOOL Signaled;
	LIST_ENTRY Waiters;
} EVENT;


/*--------------------------------------------------------
Mutexes
---------------------------------------------------------*/
 
typedef struct MUTEX {
	LIST_ENTRY WaitListHead;
	ULONG RecursionCounter;
	HANDLE Owner;
} MUTEX;

	
//
// A semaphore, containing the current number of permits, upper bounded by Limit.
//

typedef struct _SEMAPHORE {
	LIST_ENTRY WaitListHead;
	ULONG Permits;
	ULONG Limit;
} SEMAPHORE;

