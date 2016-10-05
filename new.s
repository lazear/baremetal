bits 32

; takes edx as argument
syscall_one:
	mov eax, 1
	int 0x80
	jmp $

main:
	push ebp
	mov ebp, esp
	mov edx, .string
	mov ecx, 0x47
	mov [ecx], 0xDEADBEEF

	jmp syscall_one
	jmp $

.string:
db "HELLO"
