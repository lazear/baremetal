/*
Michael Lazear, (C) 2007-2016

kernel.c
*/

#include <kernel.h>
#include <vga.h>
#include <x86.h>
#include <stdlib.h>
#include <string.h>

int ticks = 0;

extern void try();

void timer(struct regs *r) {
	ticks++;
	if (ticks%5) {
		char* buf = alloc(16);
		itoa(ticks, buf, 10);

		vga_kputs(buf, 150, 0);
	}
}


// Disable interrupts
void cli() {
	asm volatile("cli");
}

// Enable interrupts
void sti() {
	asm volatile("sti");
}

//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled
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
	4.	Todo - initialize multithreading.
	*/
	uint32_t* pagedir = k_mm_init(kernel_end);
	k_paging_init(pagedir);
	k_heap_init();

	// Start timer
	irq_install_handler(0, timer);

	// Start interrupts
	sti();

	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	vga_pretty("baremetal initialized\n", VGA_CYAN);

	for(;;);
}

