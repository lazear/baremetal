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

	vga_setcolor(VGA_COLOR(VGA_LIGHTGREY, VGA_BLACK));
	vga_clear();

	vga_puts("baremetal!\n");
	irq_install_handler(1, test);
	irq_install_handler(0, timer);
	char *s;
	int d = &kernel_initialize; // position of the kernel in memory

	itoa(s, 16, d, 8, 'a');
	vga_puts("Kernel loaded to: ");
	vga_puts(s);
	vga_putc('\n');
	vga_pretty("Red\n", VGA_RED);
	vga_pretty("Blue\n", VGA_BLUE);
	vga_pretty("Green\n", VGA_GREEN);
	vga_pretty("Cyan\n", VGA_CYAN);
	vga_pretty("Grey\n", VGA_LIGHTGREY);
	vga_pretty("Pink\n", VGA_LIGHTMAGENTA);

	char* name = "Michael Lazear, (C) 2016";


	char *dest = "Michael Lazear";
	memcpy(dest, "HE", 2);

	vga_puts(dest);

	memset(name+2, '5', 5);
	vga_puts(name);


	for(;;);
}

