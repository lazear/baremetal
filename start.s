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

%macro ISR_MACRO 1
	global isr%1
	isr%1:
		cli
		push byte %1
		jmp irqstub
%endmacro

; No error ISR macro
%macro ISR_NMACRO 1
	global isr%1
	isr%1:
		cli
		push byte 0
		push byte %1
		jmp irqstub
%endmacro

%macro IRQ_MACRO 2
	global irq%1
	irq%1:
		cli
		push byte 0
		push byte %2
		jmp irqstub
%endmacro

ISR_NMACRO 0
ISR_NMACRO 1
ISR_NMACRO 2
ISR_NMACRO 3
ISR_NMACRO 4
ISR_NMACRO 5
ISR_NMACRO 6
ISR_NMACRO 7
ISR_NMACRO 8

ISR_MACRO 9
ISR_MACRO 10
ISR_MACRO 11
ISR_MACRO 12
ISR_MACRO 13
ISR_MACRO 14
ISR_MACRO 15
ISR_MACRO 16
ISR_MACRO 17
ISR_MACRO 18
ISR_MACRO 19
ISR_MACRO 20
ISR_MACRO 21
ISR_MACRO 22
ISR_MACRO 23
ISR_MACRO 24
ISR_MACRO 25
ISR_MACRO 26
ISR_MACRO 27
ISR_MACRO 28
ISR_MACRO 29
ISR_MACRO 30
ISR_MACRO 31

IRQ_MACRO 0, 32
IRQ_MACRO 1, 33
IRQ_MACRO 2, 34
IRQ_MACRO 3, 35
IRQ_MACRO 4, 36
IRQ_MACRO 5, 37
IRQ_MACRO 6, 38
IRQ_MACRO 7, 39
IRQ_MACRO 8, 40
IRQ_MACRO 9, 41
IRQ_MACRO 10, 42
IRQ_MACRO 11, 43
IRQ_MACRO 12, 44
IRQ_MACRO 13, 45
IRQ_MACRO 14, 46
IRQ_MACRO 15, 47

extern timer
extern system_tss
 ;save all registers in the kernel-level stack of the process and switch to the kernel stack
extern k_current_process		; pointer to current process
extern kstack

# global irq0
# irq0:
#     cld
#     pushad
#     push ds
#     push es
#     push fs
#     push gs
# 	mov ax,0x10 ;load kernel segments
# 	mov ds,ax
# 	mov es,ax
# 	mov fs,ax
# 	mov gs,ax

#     mov eax,[k_current_process] 	;put the adress of the struct of CURRENT PROCESS in eax.(the CONTENT of pointer p)
#     mov [eax],esp 					;save esp in the location of esp in the CURRENT PROCESS-STRUCT.
#     lea eax,[kstack] 			; switch to the kernels own stack.
#     mov esp,eax

#     call timer

#     mov eax,[k_current_process] 	;put adress of struct of current process in eax.
#     mov esp,[eax] 					;restore adress of esp.
#     mov ebx,[eax+4]					;put content of the k-stack field into ebx.

#  ;   mov [system_tss+4],ebx 			;update system tss.
#     mov al,0x20
#     out 0x20,al
#     pop gs
#     pop fs
#     pop es
#     pop ds
#     popad
#     iretd



extern irq_handler
irqstub:
	pusha
	push ds
	push es
	push fs
	push gs

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp

	push eax
	mov eax, irq_handler
	call eax
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret

extern idt_pointer
global idt_load

idt_load:
	lidt [idt_pointer]

extern gdt_pointer
global gdt_flush

gdt_flush:
	lgdt [gdt_pointer]
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

enter_protected_mode:
	cli
	;lgdt [gdt_pointer]
	mov eax, cr0
	or al, 1
	mov cr0, eax
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
stack:
	resb SIZE