;sched.s

global trapret
trapret:

	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8		; pop int-no and errcode
	iret;



global acquire
acquire:
	mov 	ecx,	[esp+4]
	mov 	eax,	1
	xchg	eax,	[ecx]
	test 	eax,	eax   
	pause   
	jnz		acquire

	ret 

global release
release:
	mov 	ecx, 	[esp+4]		; grab the first argument off the stack
	mov     eax, 	0          	; Set the EAX register to 0.
	xchg    eax, 	[ecx]   	; Swap *ptr with 0
	ret