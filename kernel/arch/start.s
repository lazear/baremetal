;start.s
;===============================================================================
;MIT License
;Copyright (c) 2007-2016 Michael Lazear
;
;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:
;
;The above copyright notice and this permission notice shall be included in all
;copies or substantial portions of the Software.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;SOFTWARE.
;===============================================================================

; Initialize paging with 4KB pages, give control to kernel

[BITS 32]

extern kernel_initialize
extern gdt_init
extern idt_init
extern kernel_end

global entry
entry:

	mov eax, (_init_pd - VIRT)		; eax = page directory address
	mov ebx, (_init_pt - VIRT)		; ebx = page table address

	or ebx, 3
	mov [eax], ebx					; identity map 0-4MB
	xor ebx, 3

; Calculate the page directory offset
	push eax				; Save eax

	mov eax, PDE 			; Set eax = (0xC0000000 >> 22), page directory index
	mov ecx, 4				; Set muliplier = 4 (32 bits)
	mul ecx					; eax = ecx * eax
	mov ecx, eax			; move result back to ecx

	pop eax					; Restore old value of eax (Page directory)
	add eax, ecx			; Increase PD address by offset of 0xC0000000

	or ebx, 3				; Set PF_PRESENT & PF_RW flags
	mov [eax], ebx			; Use the same 0-4MB page table for 3 GB entry
	xor ebx, 3				; reset ebx

	xor ecx, ecx			; clear ecx before entering the loop

; Begin a loop to identity map the first 4 MB of virtual address space
.loop:	

	or ecx, 0x000000003 	; set PF_PRESENT, PF_RW
	mov [ebx], ecx			; set page table entry to value in ecx (phys addr)
	xor ecx, 0x00000003 	; clear the bit
	add ebx, 4				; increment by 32 bits
	add ecx, 0x1000			; increment the physical address by 4 KB
	cmp ecx, 0x00400000		; Loop until we hit 4 MB
jne .loop

	xor eax, eax
	mov eax, (_init_pd - VIRT)

	mov cr3, eax			; Eax is still our page directory address
	
	mov ecx, cr0
	or ecx, 0x80000000		; Enable paging
	mov cr0, ecx

	lea ecx, [start]		; load effective address 
	jmp ecx					; jmp to start

start:
	mov eax, stack_top 		; Establish the 16K kernel stack
	mov esp, eax
	call gdt_init			; C function, initialize GDT
	call idt_init			; C function, initialize IDT

	push kernel_end			; pass kernel end to our function
	call kernel_initialize
	jmp $


ALIGN 4
multiboot:

    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
    MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

	; Multiboot header
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM


VIRT 	equ 0xC0000000
PDE 	equ	(VIRT >> 22)
SIZE 	equ	0x4000

section .data
align 0x1000
; We will statically allocate the initial page table
; and page directory. Should take 8 KB total

global _init_pt
global _init_pd
_init_pt:				; 4 MB page table
	times 1024 dd 0
_init_pd:
	times 1024 dd 0

; We will also statically allocate a 16 Kb stack
section .bss
align 32
global stack_bottom
global stack_top

stack_bottom:
	resb SIZE
stack_top: