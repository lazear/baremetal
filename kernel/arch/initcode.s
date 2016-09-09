;initcode.s

global start
start:
	push argv
	push init
	push 0
	mov eax, 10
	int 0x80

exit:
	mov eax, 1
	int 0x80
	jmp exit

init:
	db "/init\0"

argv:
	dd init
	dd 0


