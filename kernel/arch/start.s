; start.s
; 2007-2016, Michael Lazear
; Initialize paging, move kernel to 0xC0000000
; Set up the GDT, IDT, and then hand control to 
; kernel_initialize()

[BITS 32]

extern kernel_initialize
extern gdt_init
extern idt_init
extern kernel_end

global entry
entry:

	mov ebx, (_init_pt - VIRT)		; ebx = page table address
	mov eax, (_init_pd - VIRT)		; eax = page directory address

	or ebx, 3
	mov [eax], ebx					; identity map 0-4MB
	xor ebx, 3

	; Calculate the page directory offset
	push eax			; Save eax
	mov eax, PDE 		; Set eax = (0xC0000000 >> 22)
	mov ecx, 4			; Set muliplier = 4 (32 bits)
	mul ecx				; eax = ecx * eax
	mov ecx, eax		; move result back to ecx
	pop eax				; Restore old value of eax (Page directory)
	add eax, ecx		; Increase PD address by offset of 0xC0000000

	or ebx, 3			; Set PF_PRESENT & PF_RW flags
	mov [eax], ebx		; Use the same 0-4MB page table for 3 GB entry
	xor ebx, 3			; reset ebx

	xor ecx, ecx			; clear ecx

; Begin a loop to identity map the first 4 MB of virtual address space
_id_loop:	

	or ecx, 0x000000003 	; set PF_PRESENT, PF_RW
	mov [ebx], ecx			; set page table entry to value in ecx (phys addr)
	xor ecx, 0x00000003 	; clear the bit
	add ebx, 4				; increment by 32 bits
	add ecx, 0x1000			; increment the physical address by 4 KB
	cmp ecx, 0x00400000		; Loop until we hit 4 MB
	jne _id_loop

	mov cr3, eax			; Eax is still our page directory address
	
	mov ecx, cr0
	or ecx, 0x80000000		; Enable paging
	mov cr0, ecx

	lea ecx, [start]
	jmp ecx


start:

	call gdt_init
	call idt_init

	mov eax, stack_top
	mov esp, eax

	push kernel_end
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

	VIRT equ 0xC0000000
	PDE 	equ	(VIRT >> 22)

SIZE equ	0x4000



global k_paging_load_directory
k_paging_load_directory:
	push ebp
	mov ebp, esp
	mov eax, [esp+8]
	mov cr3, eax
	mov esp, ebp
	pop ebp
	ret

global k_paging_enable
k_paging_enable:
	push ebp
	mov ebp, esp
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	mov esp, ebp
	pop ebp
	ret

section .data
align 0x1000
BootPagedirectory:
	dd	0x00000083			; Need first entry
	times (PDE - 1) dd 0	; Empty space
	dd 0x000000083			; 4mb where kernel is
	times (1024 - PDE - 1) dd 0

global _init_pt
global _init_pd
_init_pt:
	times 1024 dd 0
_init_pd:
	times 1024 dd 0

section .bss
align 32
global stack_bottom
global stack_top

stack_bottom:
	resb SIZE
stack_top: