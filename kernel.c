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


void timer(struct regs *r) {
	ticks++;
	
}


void test(struct regs *r) {

}

//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts enabled
void kernel_initialize() {

	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();

	vga_puts("baremetal!\n");
	irq_install_handler(1, test);
	irq_install_handler(0, timer);


	//char *buff = 0xC0000000;
	char *buff = alloc(10);
	//memset(buff, 0, 4096);
	vga_puts(buff);
	int d = &kernel_initialize; // position of the kernel in memory
	itoa(d, buff, 16);

	char text[100] = "Kernel loaded to: 0x";
	vga_puts(strcat(text, buff));
	vga_putc('\n');
	k_heap_init();

	vga_test();


	for(;;);
}

