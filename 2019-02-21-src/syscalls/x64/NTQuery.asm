

.code

NtQueryTimerResolution proc
 mov	r10, rcx
 mov	eax, 346  ; syscall number (0x15a)
 syscall
 ret 
NtQueryTimerResolution endp

end