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
#include <vga.h>
#include <x86.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <mutex.h>

#include <assert.h>
#include <ide.h>
#include <ext2.h>

#include <traps.h>
#include <smp.h>


//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled

extern uint32_t* KERNEL_PAGE_DIRECTORY;
uint32_t KERNEL_END = 0;

extern uint32_t _init_pt[];
extern uint32_t _init_pd[];


extern void enter_usermode(uint32_t);

mutex km = {.lock=0};


void init_message(char* message, char status) {
	vga_puts("[");
	vga_setcolor((status)? VGA_LIGHTGREEN : VGA_LIGHTRED);
	vga_puts( (status) ? " OK " : "FAIL" );
	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	printf("] %s", message);
}

void scheduler(void) {
	acquire(&km);
	printf("my id %d:%d \n", cpu->id, mp_processor_id());
	//printf("%x\n", cpu->stack);
	release(&km);
	sti();
	for(;;);
}


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
	init_message("crunchy 0.1 locked and loaded!\n", 1);
	//enter_usermode(0);
	ide_init();
	buffer_init();

	/* Initialize BSP cpu-local variables */

	//elf_objdump(ext2_open(ext2_inode(1, 12)));

	/* Parse ACPI tables for number of processors */
	int nproc = acpi_init();

	//asm volatile("mov %0, %%gs": :"r"(0x28) );


	pic_disable();

	ioapicinit();
	ioapicenable(0, 0);
	ioapicenable(0, 1);
	ioapicenable(IRQ_IDE, 0);
	ioapicenable(IRQ_IDE, 1);


	lapic_init();

	/* Acquire kernel mutex */
	acquire(&km);
	printf("Found %d processors\n", nproc);
	mp_start_ap(nproc);
		
	// Wait until all processors have been started 
	while(mp_number_of_processors() != nproc);

	printf("All processors started!\n");
	
	k_paging_map(0, 0, 0x7);
	k_paging_map(0x1000, 0x1000, 0x7);
	k_map_kernel(&_init_pd);
	enter_usermode(0x1000);

	/* Once BSP releases the initial km lock, AP's will enter scheduler */
	release(&km);

	//mutex_test();


	dprint(SEG_UCODE << 3);
	dprint(SEG_UDATA << 3);

	sti();

//	enter_usermode(0);
	scheduler();
}


