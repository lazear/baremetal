;start.s
;2007-2016, Michael Lazear
;combine ISRs, IRQ, GDT setup into a multiboot sector

[BITS 32]

extern kernel_initialize

ALIGN 4
multiboot:

	MULTIBOOT_PAGE_ALIGN	equ	1<<0
	MULTIBOOT_MEMORY_INFO	equ	1<<1
	MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
	MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
	MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


	; Multiboot header
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

SIZE equ	0x4000

global entry
entry:
	mov esp, stack + SIZE
	push ebx
	int 0x10
	call kernel_initialize
	jmp $

section .bss
align 32
stack:
	resb SIZE