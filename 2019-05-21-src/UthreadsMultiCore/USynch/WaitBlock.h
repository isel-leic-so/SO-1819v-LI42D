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
#include "UThread.h"
#include "List.h"
 
//
// Wait block used to queue requests on synchronizers.
//
typedef struct _WAIT_BLOCK {
	LIST_ENTRY Link;
	HANDLE     Thread;
} WAIT_BLOCK, *PWAIT_BLOCK;

//
// Initializes the specified wait block.
//
FORCEINLINE
VOID InitializeWaitBlock (PWAIT_BLOCK WaitBlock) {
	WaitBlock->Thread = UtSelf();
}
