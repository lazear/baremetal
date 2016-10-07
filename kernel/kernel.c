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


extern void enter_usermode(uint32_t, uint32_t);

mutex km = {.lock=0};



void scheduler(void) {
	acquire(&km);
	init_message(1,"CPU %d initialized\n", cpu->id);
	release(&km);
	for(;;);
}


int pathize(char* path) {
	printf("PATHIZE\n");
	char* pch = strtok(path, "/");
	int parent = 2;
	while(pch) {
		parent = find_inode_in_dir(pch, parent);
		printf("%s inode: %i\n", pch, parent);
		pch = strtok(NULL, "/");
	}
	return parent;
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

	sti();
	vga_init();
	init_message(1, "xiphos kernel locked and loaded!\n");

	ide_init();
	buffer_init();

	build_ksyms();

//	elf_execute("lass.elf");
	elf_execute("lquad.elf");
	//printf("ksym_find() %s\n", ksym_find(0xC0102F00));




// #define SMP 0

// 	if(SMP) {
// 		/* Parse ACPI tables for number of processors */
// 		int nproc = acpi_init();
// 		/* Acquire kernel mutex, otherwise things will get out of hand quickly */
// 		acquire(&km);

// 		init_message(1, "Found %d processors\n", nproc);
// 		pic_disable();
// 		lapic_init();
// 		ioapicinit();
// 		ioapicenable(0, 0);
// 		ioapicenable(0, 1);
// 		ioapicenable(IRQ_IDE, 0);
// 		ioapicenable(IRQ_IDE, 1);

// 		/* Start all AP's */
// 		mp_start_ap(nproc);
			
// 		/* Wait until all processors have been started  */
// 		while(mp_number_of_processors() != nproc);

// 		init_message(1, "All processors started!\n");


// 		/* Once BSP releases the initial km lock, AP's will enter scheduler */
// 		pushcli();
// 		release(&km);
// 	}
	scheduler();
}


