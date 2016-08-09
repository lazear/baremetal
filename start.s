;start.s
;2007-2016, Michael Lazear
;combine ISRs, IRQ, GDT setup into a multiboot sector

bits 32

global entry
extern kernel_initialize

entry:
	jmp start

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

start:
	mov esp, stack + SIZE
	push ebx
	call kernel_initialize
	jmp $

section .bss
align 32
stack:
	resb SIZE