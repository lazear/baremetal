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


void test(struct regs *r) {
	kprintd("Ticks: ", ticks);
	asm volatile("sti");
}


//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts enabled
void kernel_initialize(uint32_t kernel_end) {




	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	uint32_t* pagedir = k_mm_init(kernel_end);

	k_paging_init_test(pagedir);
	//vga_puts("baremetal!\n");

	irq_install_handler(0, timer);
	kprintx("dir: ", pagedir);

	k_paging_map_block( 0xB8000, 0x00400000, 3 );
	k_paging_map_block( 0x8000, 0x00480000, 3 );
	char* ptr = 0x00400000;


	vga_puts(ptr);

	mm_test();
	for(;;);
}

