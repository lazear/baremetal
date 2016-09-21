/*
kernel.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/

#include <kernel.h>
#include <crunch.h>
#include <vga.h>
#include <x86.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <mutex.h>
#include <proc.h>
#include <assert.h>
#include <ide.h>
#include <ext2.h>
#include <elf.h>

static mutex key_mutex = { .lock = 0 };



extern void scheduler();
void scheduler() {}
extern void switch_to_user(void* (fn)(), uint32_t esp);
//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled

extern uint32_t* KERNEL_PAGE_DIRECTORY;
uint32_t KERNEL_END = 0;

extern uint32_t _init_pt[];
extern uint32_t _init_pd[];
extern uint32_t _binary_ap_entry_start[];
extern uint32_t _binary_ap_entry_end[];

extern uint32_t stack_top;
extern uint32_t stack_bottom;


#define dprint(e) (printf("%s: %x\n", #e, e) )

void kernel_initialize(uint32_t kernel_end) {

	/*
	1.	Initialize physical memory manager, with private heap @ kernel_end to 2MB
			- Bitmap has the first 2MB (512 bits) marked as used, will not be allocated
			- Allows k_page_alloc()
	2.	Initialize paging, passing the reserved first page directory address
	3.	Initialize heap management with malloc, free, and sbrk.
			- Utilizes both k_page_alloc() and k_paging_map() to generate a continous
				virtual address space.
			- Public heap starts at 3GB. This should be changed when higher half is implemented
	4.	Todo - initialize:
			* APIC/ACPI/Multiple processors
			* multitasking.
	*/

	KERNEL_END = kernel_end;
	k_mm_init(kernel_end);
	k_heap_init();
	k_paging_init(_init_pd);

	keyboard_install();
	timer_init();
	syscall_init();

	sti();
	vga_init();
	vga_pretty("crunchy 0.1 locked and loaded!\n", VGA_LIGHTGREEN);

	ide_init();
	buffer_init();



	//acpi_init();
	//ioapicinit();
	//ioapicenable(0, 0);
	//ioapicenable(0, 1);

	//lapic_init();


	//ap_entry_init();
	//lapic_start_AP(1);

	elf_load();

	for(;;);
}


