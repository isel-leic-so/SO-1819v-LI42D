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

#include <crtdbg.h>
#include <stdio.h>
#include "UThreadInternal.h"

//////////////////////////////////////
//
// UThread internal state variables.
//

//
// The number of existing user threads.
//
 
VOID UtYieldInternal(PUPROCESSOR cp);
// The Processors' array
static PUPROCESSOR processors;
static DWORD processorsNumber;



VOID SPEnter(PSPINLOCK sl) {
	while (TRUE) {
		while (*sl == 1);
		LONG old = InterlockedExchange(sl, 1);
		if (old ==0)	
			break;
	}
}

VOID SPLeave(PSPINLOCK sl) {
	InterlockedExchange(sl, 0);
}



 

////////////////////////////////////////////////
//
// Forward declaration of internal operations.
//

HANDLE UtCreateInternal(UT_FUNCTION Function, UT_ARGUMENT Argument, PUPROCESSOR p);

//
// The trampoline function that a user thread begins by executing, through
// which the associated function is called.
//
static
VOID InternalStart ();


VOID  InternalStart0();

#ifdef _WIN64
//
// Performs a context switch from CurrentThread to NextThread.
// In x64 calling convention CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall  ContextSwitch64 (PUTHREAD CurrentThread, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// In x64 calling convention  CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall InternalExit64 (PUTHREAD Thread, PUTHREAD NextThread);

#define ContextSwitch ContextSwitch64
#define InternalExit InternalExit64

#else

static
VOID __fastcall ContextSwitch32 (PUPROCESSOR p, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX
// and NextThread in EDX.
//
static
VOID __fastcall InternalExit32 (PUPROCESSOR p, PUTHREAD NextThread);

#define ContextSwitch ContextSwitch32
#define InternalExit InternalExit32
#endif

////////////////////////////////////////
//
// UThread inline internal operations.
//

//
// Returns and removes the first user thread in the ready queue. If the ready
// queue is empty, the main thread is returned.
//
static
FORCEINLINE
PUTHREAD ExtractNextReadyThread (PUPROCESSOR cp) {
	return IsListEmpty(&cp->ReadyQueue) 
		 ? cp->MainThread 
		 : CONTAINING_RECORD(RemoveHeadList(&cp->ReadyQueue), UTHREAD, Link);
}

//
// Schedule a new thread to run
//
static
FORCEINLINE
VOID Schedule (PUPROCESSOR p) {
	PUTHREAD NextThread;
    NextThread = ExtractNextReadyThread(p);
	SPLeave(&p->Lock);
	ContextSwitch(p, NextThread);
}

///////////////////////////////
//
// UThread public operations.
//

static PUPROCESSOR GetTargetProcessor() {
	DWORD minThreads = processors[0].NumberOfThreads;
	DWORD minProcessor = 0;
	for (DWORD i = 1; i < processorsNumber; ++i) {
		if (processors[i].NumberOfThreads < minThreads) {
			minThreads = processors[i].NumberOfThreads;
			minProcessor = i;
		}
	}
	return processors + minProcessor; 
}

static PUPROCESSOR GetCurrentProcessor() {
	DWORD tid = GetCurrentThreadId();
	for (DWORD i = 0; i < processorsNumber; ++i) {
		if (processors[i].SysThreadId == tid) return &processors[i];
	}
	_ASSERT(0); // Shouldn't read this line!
	return NULL;
}

static VOID IdleFunc(UT_ARGUMENT arg) {
	PUPROCESSOR currProcessor = GetCurrentProcessor();
#ifdef VERBOSE
	printf("start idle for processor %d\n", currProcessor->Id);
#endif
	for (; ; ) {
#ifdef VERBOSE
		printf("Enter idle thread on processor %d\n", currProcessor->Id);
#endif
#ifdef WAIT_EMPTYQUEUE
		WaitForSingleObject(currProcessor->NotEmptyReadyQueue, INFINITE);
#endif
#ifdef VERBOSE
		printf("Leave idle thread on processor %d\n", currProcessor->Id);
#endif
		if (currProcessor->NumberOfThreads <= 1) 
			break;
		UtYieldInternal(currProcessor);
	}
}

static DWORD WINAPI ProcFunc(LPVOID arg) {
	PUPROCESSOR cp = GetCurrentProcessor();
	UTHREAD Thread; // Represents the underlying operating system thread.

	//
	// There can be only one scheduler instance running.
	//
	_ASSERTE(cp->RunningThread == NULL);
#ifdef VERBOSE
	printf("processor %d started!\n", cp->Id);
#endif
	//
	// At least one user thread must have been created before calling run.
	//
	if (IsListEmpty(&cp->ReadyQueue)) {
#ifdef VERBOSE
		printf("processor %d stopped!\n", cp->Id);
#endif
		return 0;
	}

	//
	// Switch to a user thread.
	//
	cp->MainThread = &Thread;
	cp->RunningThread = cp->MainThread;
	SPEnter(&cp->Lock);
	Schedule(cp);
#ifdef VERBOSE
	printf("processor %d stopped!\n", cp->Id);
#endif
	//
	// When we get here, there are no more runnable user threads.
	//
	_ASSERT(IsListEmpty(&cp->ReadyQueue));
	//_ASSERT(cp->NumberOfThreads == 0);

	
	cp->RunningThread = NULL;
	cp->MainThread = NULL;
	return 0;
}

static void InitProcessor(PUPROCESSOR p, DWORD id) {
	ZeroMemory(p, sizeof(UPROCESSOR));
	InitializeListHead(&p->ReadyQueue);
	
	DWORD threadId;
	p->Id = id;
	p->NotEmptyReadyQueue = CreateEvent(NULL, FALSE, FALSE, NULL);
	p->SysThread = CreateThread(NULL, 0, ProcFunc, NULL, CREATE_SUSPENDED, &threadId);
	p->SysThreadId = threadId;
 
	UtCreateInternal(IdleFunc, p, p);
}

static void DestroyProcessor(PUPROCESSOR p) {
	CloseHandle(p->NotEmptyReadyQueue);
	 
	CloseHandle(p->SysThread);
}

//
// Initialize the scheduler.
// This function must be the first to be called. 
// Initialization od Multi-Uthread data structures
//
VOID UtInit() {
	// Dynamically allocate uthread Process descriptors array
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	processorsNumber = 2; // si.dwNumberOfProcessors;
	processors = (PUPROCESSOR)malloc(sizeof(UPROCESSOR)*processorsNumber);
	for (DWORD i = 0; i < processorsNumber; ++i) {
		InitProcessor(processors + i, i);
	}
}


VOID UtRun() {
	for (DWORD i = 0; i < processorsNumber; ++i) {
		ResumeThread(processors[i].SysThread);
	}
	for (DWORD i = 0; i < processorsNumber; ++i) {
		WaitForSingleObject(processors[i].SysThread, INFINITE);
	}
#ifdef VERBOSE
	printf("Start Run!\n");
#endif
}
//
// Cleanup all UThread internal resources.
//
VOID UtEnd() {
	for (DWORD i = 0; i < processorsNumber; ++i) {
		DestroyProcessor(processors + i);
	}
}



//
// Terminates the execution of the currently running thread. All associated
// resources are released after the context switch to the next ready thread.
//
VOID UtExit () {
	PUPROCESSOR cp = GetCurrentProcessor();
	SPEnter(&cp->Lock);
	cp->NumberOfThreads -= 1;	
	PUTHREAD nextThread = ExtractNextReadyThread(cp);
	SPLeave(&cp->Lock);
	InternalExit(cp, nextThread);
	_ASSERTE(!"Supposed to be here!");
}

//
// Relinquishes the processor to the first user thread in the ready queue.
// If there are no ready threads, the function returns immediately.
//
VOID UtYieldInternal(PUPROCESSOR cp) {
	SPEnter(&cp->Lock);
	if (!IsListEmpty(&cp->ReadyQueue)) {
		InsertTailList(&cp->ReadyQueue, &cp->RunningThread->Link);
		Schedule(cp);
		return;
	}
	SPLeave(&cp->Lock);
}

//
// Relinquishes the processor to the first user thread in the ready queue.
// If there are no ready threads, the function returns immediately.
//
VOID UtYield () {
	PUPROCESSOR cp = GetCurrentProcessor();
	UtYieldInternal(cp);
}

//
// Returns a HANDLE to the executing user thread.
//
HANDLE UtSelf () {
	PUPROCESSOR cp = GetCurrentProcessor();
	return (HANDLE)cp->RunningThread;
}




//
// Halts the execution of the current user thread.
//
VOID UtDeactivate() {
	PUPROCESSOR cp = GetCurrentProcessor();
	SPEnter(&cp->Lock);
	Schedule(cp);
}


//
// Places the specified user thread at the end of the ready queue, where it
// becomes eligible to run.
//
static VOID UtActivateInternal(HANDLE ThreadHandle, PUPROCESSOR cp) {
	
	SPEnter(&cp->Lock);
	InsertTailList(&cp->ReadyQueue, &((PUTHREAD)ThreadHandle)->Link);
	SPLeave(&cp->Lock);
}


//
// Places the specified user thread at the end of the ready queue, where it
// becomes eligible to run.
//
VOID UtActivate (HANDLE ThreadHandle) {
	UtActivateInternal(ThreadHandle, ((PUTHREAD)ThreadHandle)->Processor);
}

///////////////////////////////////////
//
// Definition of internal operations.
//

//
// The trampoline function that a user thread begins by executing, through
// which the associated function is called.
//
VOID InternalStart () {
	PUPROCESSOR cp = GetCurrentProcessor();
	cp->RunningThread->Function(cp->RunningThread->Argument);
	UtExit(); 
}


//
// Frees the resources associated with Thread..
//
VOID __fastcall CleanupThread (PUPROCESSOR p) {
	PUTHREAD Thread = p->RunningThread;
	free(Thread->Stack);
	free(Thread);
}

//
// functions with implementation dependent of X86 or x64 platform
//

#ifndef _WIN64

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreateInternal(UT_FUNCTION Function, UT_ARGUMENT Argument, PUPROCESSOR p) {
	PUTHREAD Thread;

	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	Thread = (PUTHREAD)malloc(sizeof(UTHREAD));
	Thread->Stack = (PUCHAR)malloc(STACK_SIZE);
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 0, STACK_SIZE);

	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;

	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+
	// | 0x00000000 |    <- Highest word of a thread's stack space
	// +============+       (needs to be set to 0 for Visual Studio to
	// |  RetAddr   | \     correctly present a thread's call stack).
	// +------------+  |
	// |    EBP     |  |
	// +------------+  |
	// |    EBX     |   >   Thread->ThreadContext mapped on the stack.
	// +------------+  |
	// |    ESI     |  |
	// +------------+  |
	// |    EDI     | /  <- The stack pointer will be set to this address
	// +============+       at the next context switch to this thread.
	// |            | \
	// +------------+  |
	// |     :      |  |
	//       :          >   Remaining stack space.
	// |     :      |  |
	// +------------+  |
	// |            | /  <- Lowest word of a thread's stack space
	// +------------+       (Thread->Stack always points to this location).
	//

	Thread->ThreadContext = (PUTHREAD_CONTEXT)(Thread->Stack +
		STACK_SIZE - sizeof(ULONG) - sizeof(UTHREAD_CONTEXT));

	//
	// Set the thread's initial context by initializing the values of EDI,
	// EBX, ESI and EBP (must be zero for Visual Studio to correctly present
	// a thread's call stack) and by hooking the return address.
	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->EDI = 0x33333333;
	Thread->ThreadContext->EBX = 0x11111111;
	Thread->ThreadContext->ESI = 0x22222222;
	Thread->ThreadContext->EBP = 0x00000000;
	Thread->ThreadContext->RetAddr = InternalStart;


	//
	// Ready the thread.
	//
	Thread->Processor = p;
	InterlockedIncrement(&p->NumberOfThreads);
	UtActivateInternal((HANDLE)Thread, p);


	return (HANDLE)Thread;
}
//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate32 (UT_FUNCTION Function, UT_ARGUMENT Argument) {
	
	PUPROCESSOR cp = GetTargetProcessor();
	return UtCreateInternal(Function, Argument, cp);
}

//
// Performs a context switch from CurrentThread to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked) 
VOID __fastcall ContextSwitch32 (PUPROCESSOR p, PUTHREAD NextThread) {
	__asm {
		// Switch out the running CurrentThread, saving the execution context on the thread's own stack.   
		// The return address is atop the stack, having been placed there by the call to this function.
		//
		push	ebp
		push	ebx
		push	esi
		push	edi
		//
		// Save ESP in CurrentThread->ThreadContext.
		//
		mov		esi, dword ptr [ecx].RunningThread
		mov		dword ptr [esi].ThreadContext, esp
		//
		// Set NextThread as the running thread.
		//
		mov     dword ptr[ecx].RunningThread, edx
		//
		// Load NextThread's context, starting by switching to its stack, where the registers are saved.
		//
		mov		esp, dword ptr [edx].ThreadContext

		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		//
		// Jump to the return address saved on NextThread's stack when the function was called.
		//
		ret
	}
}

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked)
VOID __fastcall InternalExit32 (PUPROCESSOR p, PUTHREAD NextThread) {
	__asm {

		
		
		//
		// Load NextThread's stack pointer before calling CleanupThread(): making the call while
		// using CurrentThread's stack would mean using the same memory being freed -- the stack.
		//
		mov		esp, dword ptr [edx].ThreadContext
		mov		esi, ecx
		mov		edi, edx
		call    CleanupThread

		//
		// Set NextThread as the running thread.
		//
		mov     dword ptr[esi].RunningThread, edi

		//
		// Finish switching in NextThread.
		//
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret
	}
}

#else

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate64 (UT_FUNCTION Function, UT_ARGUMENT Argument) {
	PUTHREAD Thread;
	
	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	Thread = (PUTHREAD) malloc(sizeof (UTHREAD));
	Thread->Stack = (PUCHAR) malloc(STACK_SIZE);
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 0, STACK_SIZE);

	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;

	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+  <- Highest word of a thread's stack space
	// | 0x00000000 |    (needs to be set to 0 for Visual Studio to
	// +------------+      correctly present a thread's call stack).   
	// | 0x00000000 |  \
	// +------------+   |
	// | 0x00000000 |   | <-- Shadow Area for Internal Start 
	// +------------+   |
	// | 0x00000000 |   |
	// +------------+   |
	// | 0x00000000 |  /
	// +============+       
	// |  RetAddr   | \    
	// +------------+  |
	// |    RBP     |  |
	// +------------+  |
	// |    RBX     |   >   Thread->ThreadContext mapped on the stack.
	// +------------+  |
	// |    RDI     |  |
	// +------------+  |
	// |    RSI     |  |
	// +------------+  |
	// |    R12     |  |
	// +------------+  |
	// |    R13     |  |
	// +------------+  |
	// |    R14     |  |
	// +------------+  |
	// |    R15     | /  <- The stack pointer will be set to this address
	// +============+       at the next context switch to this thread.
	// |            | \
	// +------------+  |
	// |     :      |  |
	//       :          >   Remaining stack space.
	// |     :      |  |
	// +------------+  |
	// |            | /  <- Lowest word of a thread's stack space
	// +------------+       (Thread->Stack always points to this location).
	//

	Thread->ThreadContext = (PUTHREAD_CONTEXT) (Thread->Stack +
		STACK_SIZE -sizeof (UTHREAD_CONTEXT)-sizeof(ULONGLONG)*5);

	//
	// Set the thread's initial context by initializing the values of 
	// registers that must be saved by the called (R15,R14,R13,R12, RSI, RDI, RBCX, RBP)
	
	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->R15 = 0x77777777;
	Thread->ThreadContext->R14 = 0x66666666;
	Thread->ThreadContext->R13 = 0x55555555;
	Thread->ThreadContext->R12 = 0x44444444;	
	Thread->ThreadContext->RSI = 0x33333333;
	Thread->ThreadContext->RDI = 0x11111111;
	Thread->ThreadContext->RBX = 0x22222222;
	Thread->ThreadContext->RBP = 0x11111111;		
	Thread->ThreadContext->RetAddr = InternalStart;

	//
	// Ready the thread.
	//
	NumberOfThreads += 1;
	UtActivate((HANDLE)Thread);
	
	return (HANDLE)Thread;
}




#endif