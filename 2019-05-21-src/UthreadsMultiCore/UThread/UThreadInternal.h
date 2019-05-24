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
#include <assert.h>
//
// The data structure representing the layout of a thread's execution context
// when saved in the stack.
//



#ifdef _WIN64
//RBX, RBP, RDI, RSI, R12, R13, R14, and R15 
typedef struct _UTHREAD_CONTEXT_64 {
	ULONGLONG R15;
	ULONGLONG R14;
	ULONGLONG R13;
	ULONGLONG R12;
	ULONGLONG RSI;
	ULONGLONG RDI;
	ULONGLONG RBX;
	ULONGLONG RBP;
	VOID (*RetAddr)();
} UTHREAD_CONTEXT_64, *PUTHREAD_CONTEXT_64;

#define PUTHREAD_CONTEXT PUTHREAD_CONTEXT_64
#define UTHREAD_CONTEXT UTHREAD_CONTEXT_64
#else
typedef struct _UTHREAD_CONTEXT_32 {
	ULONG EDI;
	ULONG ESI;
	ULONG EBX;
	ULONG EBP;
	VOID(*RetAddr)();
} UTHREAD_CONTEXT_32, *PUTHREAD_CONTEXT_32;

#define PUTHREAD_CONTEXT PUTHREAD_CONTEXT_32
#define UTHREAD_CONTEXT UTHREAD_CONTEXT_32
#endif

//
// The descriptor of a user thread, containing an intrusive link (through which
// the thread is linked in the ready queue), the thread's starting function and
// argument, the memory block used as the thread's stack and a pointer to the
// saved execution context.
//

typedef struct Processor *PUPROCESSOR;

typedef struct _UTHREAD {
	PUTHREAD_CONTEXT ThreadContext;
	LIST_ENTRY       Link;
	PUPROCESSOR		 Processor;
	UT_FUNCTION      Function;   
	UT_ARGUMENT      Argument; 
	PUCHAR           Stack;
} UTHREAD, *PUTHREAD;



//
// The fixed stack size of a user thread.
//
#define STACK_SIZE (8 * 4096)

//
// New stuff for multi processor version
//
typedef  LONG  SPINLOCK, *PSPINLOCK;


VOID SPEnter(PSPINLOCK sl);
VOID SPLeave(PSPINLOCK sl);
	 
//
// In the multi CPU version a new type Processor is defined
// Each processor has its own Ready Queue and Windows
//
typedef struct Processor {
	LIST_ENTRY ReadyQueue;		// Processor schedule queue
	//CRITICAL_SECTION Lock;		// Lock for safe processor access
	SPINLOCK Lock;
	HANDLE SysThread;			// Windows thread for process
	DWORD  SysThreadId;			// Processor thread id
	DWORD Id;					// Processor id for enumeration processes
	HANDLE NotEmptyReadyQueue;	// WakeUp Idle thread
	PUTHREAD RunningThread;		// Running thread on this processor
	// The user thread proxy of the underlying operating system thread. This
	// thread is switched back in when there are no more runnable user threads,
	// causing the scheduler to exit.
	PUTHREAD MainThread;
	DWORD NumberOfThreads;
} UPROCESSOR;