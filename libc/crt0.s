
section .text
global _start
extern main
extern _init

_start:
	mov ebp, 0
	push ebp
	push ebp
	mov ebp, esp

	call main

	mov eax, 0
	int 0x80

size equ $-$$
