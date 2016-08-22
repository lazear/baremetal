;start.s
;2007-2016, Michael Lazear
;combine ISRs, IRQ, GDT setup into a multiboot sector

[BITS 32]

extern kernel_initialize
extern gdt_init
extern idt_init
extern kernel_end

global entry
entry:

	call gdt_init
	call idt_init
	call enableA20
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

SIZE equ	0x4000


extern idt_pointer
global idt_load

idt_load:
	lidt [esp+4]

extern gdt_pointer
global gdt_flush

gdt_flush:
	lgdt [esp+4]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush
flush:
	ret

global tss_flush
tss_flush:
	mov ax, 0x2B
	ltr ax
	ret

enableA20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret


global read_stack_pointer
read_stack_pointer:
	push ebp
	mov ebp, esp

	mov eax, esp
	;add eax, 8
	pop ebp
	ret

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

global k_read_cr3
k_read_cr3:
	push ebp
	mov ebp, esp
	mov eax, cr3
	mov esp, ebp
	pop ebp
	ret




section .bss
align 32

stack_bottom:
	resb SIZE
stack_top: