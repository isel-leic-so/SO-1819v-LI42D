

.code

extrn RunningThread : 
extrn CleanupThread: proc

;-----------------------------------------------------------
;ContextSwitch64 (PUTHREAD CurrentThread, PUTHREAD NextThread) 
;-----------------------------------------------------------
ContextSwitch64 proc
 
	;
	; Switch out the running CurrentThread, saving the execution context on the thread's own stack.   
	; The return address is atop the stack, having been placed there by the call to this function.
	;
	push	rbp
	push	rbx
	push	rdi
	push	rsi
	push	r12
	push	r13
	push	r14
	push	r15
	;
	; Save ESP in CurrentThread->ThreadContext
	;
	mov		qword ptr [rcx], rsp

	;
	; Set NextThread as the running thread.
	;
	mov     RunningThread, rdx

	;
	; Load NextThread's context, starting by switching to its stack, where the registers are saved.
	;
	mov		rsp, qword ptr [rdx]

	pop		r15
	pop		r14
	pop		r13
	pop		r12
	pop		rsi
	pop		rdi
	pop		rbx
	pop		rbp

	;
	; Jump to the return address saved on NextThread's stack when the function was called.
	;
	ret

ContextSwitch64 endp

;------------------------------------------------------------------------------ 
;VOID  InternalExit64 (PUTHREAD CurrentThread, PUTHREAD NextThread) 
;-----------------------------------------------------------------------------

InternalExit64 proc	 
	;
	; Set NextThread as the running thread.
	;
	mov     RunningThread, rdx
		
	;
	; Load NextThread's stack pointer before calling CleanupThread(): making the call while
	; using CurrentThread's stack would mean using the same memory being freed -- the stack.
	;
	mov		rsp, qword ptr [rdx]
	sub		rsp, 32
	call    CleanupThread
	add		rsp, 32

	;
	; Finish switching in NextThread.
	;
	pop		r15
	pop		r14
	pop		r13
	pop		r12
	pop		rsi
	pop		rdi
	pop		rbx
	pop		rbp
	ret

InternalExit64 endp
end