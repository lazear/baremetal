; switch_to_user.s

[BITS 32]

global switch_to_user
switch_to_user:
	cli
	mov edx, [esp+4]
	mov eax, [esp+8]

	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	;mov eax, esp
	push 0x23		; push Ring3_SS
	push eax 		; push Ring3_ESP
	pushf			; push flags
	pop eax			; pop into EAX
	or eax, 0x200	; OR 0x200 (int enabled)
	push eax 		; push back into flags reg
	push 0x1B		; user code segment, bottom two bits (3) set for Ring 3
	push jump
	iret
jump:
	jmp edx
