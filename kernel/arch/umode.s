
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



asmitoa:
	mov esi, datastor 	; buffer
	xor edx, edx	
	mov ecx, 0x10
.loop:
	cmp eax, 0
	jz .done

	lodsb 
	div ecx
	add dl, '0'
	mov [esi], edx

	jmp .loop

.done:
	ret


datastor resb 32

global test
test:
	mov esi, .string
	mov eax, 0xDEADFEEF
	;lcall asmitoa

.loop:
	cld
	lodsw

	cmp al, 'o fr'
	jnz .loop
	mov eax, 1
	mov edx, datastor
	int 0x80
	jmp $

.string db 'Hello from userland?', 0

global enter_usermode
enter_usermode:
	cli

	mov ax, 0x2B
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push 0x2B		; SS3

	push 0x1000				; ESP3

	push 0x202
	push 0x23		; CS
	push test

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