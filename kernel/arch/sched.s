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


global exchange
exchange:
	push ebp
	push ebx
	mov eax, [esp+8]
	xchg [esp+4], eax

	pop ebx
	pop ebp
	ret