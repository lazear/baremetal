
section .text
global _start
extern main
extern _init
_start:
	mov ebp, 0
	push ebp
	push ebp
	mov ebp, esp
	
	push esi
	push edi
	call _init
	pop edi
	pop esi

	call main
	mov edi, eax
	int 0x80
size equ $-$$
