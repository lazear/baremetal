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
	k_paging_map_block(dir, 0xDEAD0000, 0xDEAD0000, 0x3);
	//k_paging_map_block(dir, 0xB0000000, 0xB0000000, 0x3);

	char* ptr = 0xDEADFF00;
	*ptr = 'A';
	vga_puts(ptr);

	for(;;);
}

