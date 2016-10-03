
;umode.s
[BITS 32]

global gdt_flush
gdt_flush:

	jmp 0x08:.csjmp

.csjmp:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret

global test
test:
	push ebp
	mov ebp, esp
	sub esp, 12
	mov esi, esp
	mov dx, 10
	mov cl, 'A'
	mov [esi], cl

	mov eax, 1
	mov edx, esi

	int 0x80

	mov eax, 2202
	int 0x80

	jmp $

extern printf
global enter_usermode
enter_usermode:
	;push ebp
	mov ebp, esp
	cli

	mov ax, SEG_UDATA | 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push SEG_UDATA | 3	; push ss3

	mov ecx, [ebp+8]
	push ecx			; push esp3

	pushf 				; push flags onto stack
	pop eax				; pop into eax
	or eax, 0x200		; set IF (enable interrupts)
	push eax			; push eflags
	push SEG_UCODE | 3	; push CS, requested priv. level = 3

	xor eax, eax
	mov eax, [ebp+4]
	push eax

	iret

SEG_UCODE	equ 4 << 3
SEG_UDATA	equ 5 << 3

; .globl enter_usermode
; .globl gdt_flush

; test:
; 	jmp test

; enter_usermode:
; 	cli
; 	movw $0x2B, %ax
; 	movw %ax, %ds
; 	movw %ax, %es
; 	movw %ax, %fs
; 	movw %ax, %gs

; 	pushw 0
; 	pushl $0x2B
; 	movl %esp, %eax
; 	pushl %eax
; 	pushl $0x202
; 	pushl $0x23
; 	pushl $test
; 	//iret
; 	ljmp $0x23, $test

; .loop:
; 	jmp .loop

; gdt_flush:
; 	ljmp $0x8, $.csjmp

; .csjmp:
; 	movw $0x10, %ax
; 	movw %ax, %ds
; 	movw %ax, %es
; 	movw %ax, %fs
; 	movw %ax, %gs
; 	ret