;syscall.asm
[BITS 32]

extern syscall

global syscall_handler
syscall_handler:

	push 0
	push 0x80
	cli
	
	pusha
	push ds
	push es
	push fs
	push gs

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp

	push eax
	call syscall
	

	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret