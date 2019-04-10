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

#ifdef UTHREAD_DLL
#define UTHREAD_API __declspec(dllexport)
#else
#define UTHREAD_API  __declspec(dllimport)
#endif

#include <Windows.h>

typedef VOID * UT_ARGUMENT;
typedef VOID (*UT_FUNCTION)(UT_ARGUMENT);

#ifdef __cplusplus
extern "C" {
#endif


//
// Initialize the scheduler.
// This function must be the first to be called. 
//
UTHREAD_API
VOID UtInit ();

//
// Cleanup all UThread internal resources.
//
UTHREAD_API
VOID UtEnd ();

//
// Run the user threads. The operating system thread that calls this function
// performs a context switch to a user thread and resumes execution only when
// all user threads have exited.
//
UTHREAD_API
VOID UtRun ();

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//

#ifdef _WIN64
#define UtCreate UtCreate64
#else
#define UtCreate UtCreate32
#endif

// Uncomment next line when build for x64

UTHREAD_API
HANDLE UtCreate (UT_FUNCTION Function, UT_ARGUMENT Argument);

//
// Terminates the execution of the currently running thread. All associated
// resources are released after the context switch to the next ready thread.
//
UTHREAD_API
VOID UtExit ();

//
// Relinquishes the processor to the first user thread in the ready queue.
// If there are no ready threads, the function returns immediately.
//
UTHREAD_API
VOID UtYield ();

//
// Returns a HANDLE to the executing user thread.
//
UTHREAD_API
HANDLE UtSelf ();

//
// Halts the execution of the current user thread.
//
UTHREAD_API
VOID UtDeactivate ();

//
// Places the specified user thread at the end of the ready queue, where it
// becomes eligible to run.
//
UTHREAD_API
VOID UtActivate (HANDLE ThreadHandle);

UTHREAD_API
BOOL UtAlive(HANDLE ThreadHandle);

UTHREAD_API
BOOL UtJoin(HANDLE ThreadHandle);

#ifdef __cplusplus
} // extern "C"
#endif
