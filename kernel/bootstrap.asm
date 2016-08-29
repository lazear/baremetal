; Fat12 Bootloader

[BITS 16]                       ; We need 16-bit intructions for Real mode

[ORG 0x7C00]                    ; The BIOS loads the boot sector into memory location 0x7C00

jmp word start            ; Load the OS Kernel

	;----------Fat 12 Header junk----------;

	;----------Bootsector Code----------;
start:
	mov ax, 0x4F02          ; set VBE mode
	mov bx, 0x4118          ; Bit 14 is set(Linear frame buffer) 15 is clear
	int 0x10

	cmp ax, 0x004F
	jne .error

.print:
	mov bp, .string
	mov ah, 0x13
	mov al, 0x01
	mov bh, 0x00
	mov bl, 0x1F
	mov cx, .strlen
	int 0x10
	jmp $


.error:
	mov ah, 09h
	mov al, 'C'
	mov bh, 0
	mov bl, 0xA;
	mov cx, 2
	int 0x10

.string	db 13, 'Hello, World!'
.strlen equ $-.string

times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                   	; Last 2 bytes = Boot sector identifyer