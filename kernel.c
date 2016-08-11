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
	
}


void test(struct regs *r) {
	kprintd("Ticks: ", ticks);
	asm volatile("sti");
}

void bitshift(int x) {
	kprintx("", (x >> 12)<<2);
}

//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts enabled
void kernel_initialize(int ebx) {
	//paging_init();
	k_paging_init();
	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	


	vga_puts("baremetal!\n");
	irq_install_handler(1, test);
	irq_install_handler(0, timer);

	k_heap_init();
		// throwing system exception when calling ftoa(atof(""))
		//ftoa(c);
	//intten();

	//k_map(0xB8000, 0xC0000000, 3);

	

	uint32_t* dir = k_paging_get_dir();
	k_paging_map_block(dir, 0xDEADBEEF, 0xDEADBEEF, 0x3);

	load_page_directory(dir);
	flush_tlb();

	int* pt = dir[0xDEADBEEF >> 22];
	int* pp = pt[(0xDEADBEEF >> 12) & 0x3FF];
	kprintx("", pp );

	char* ptr = 0xDEADBEEF;
	*ptr = 'A';

	for(;;);
}

