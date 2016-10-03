
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

	mov edi, [ebp+4]	; argc
	mov esi, [ebp+8]	; argv


	call crt_initialize
	
	push esi			; argv
	push edi			; argc	

	call main

	jmp $
	add esp, 8			; cleanup the stack

	mov esp, ebp
	pop ebp


	ret 
