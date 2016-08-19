;sched.s

extern scheduler
global sched
sched:
	

	push ebp
	push ebx
	push esi
	push edi

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, esp
	push eax
	
	call scheduler
	mov esp, eax

	pop edi
	pop esi
	pop ebx
	pop ebp

	ret

extern pushcli
extern popcli

global acquire
acquire:
	call pushcli
	mov 	ecx,	[esp+4]
	mov 	eax,	1
	xchg	eax,	[ecx]
	test 	eax,	eax      
	jnz		acquire

	ret 

global release
release:
	mov 	ecx, 	[esp+4]		; grab the first argument off the stack
	mov     eax, 	0          	; Set the EAX register to 0.
	xchg    eax, 	[ecx]   	; Swap *ptr with 0
	call popcli
	ret