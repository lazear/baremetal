
section .text
global _start
extern main
extern crt_initialize

; Until we get multi-threading and exit() working, this will have to do
_start:
	;mov ebp, 0
	;push ebp
	push ebp
	mov ebp, esp

	mov esi, [ebp+8]	; argc
	mov edi, [ebp+12]	; argv
	
	push esi
	push edi

	call crt_initialize


	call main

	add esp, 8			; cleanup the stack

	mov esp, ebp
	pop ebp


	ret 
