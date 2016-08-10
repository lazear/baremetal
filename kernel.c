/*
Michael Lazear, (C) 2007-2016

kernel.c
*/

#include <kernel.h>
#include <vga.h>
#include <x86.h>


void test(struct regs *r) {
	vga_puts("TEST");
}


void kernel_initialize() {

	vga_setcolor(0x2);
	vga_clear();

	vga_puts("Hello, World from baremetal!\n");
	vga_puts("Newline test");
	gdt_init();
	irq_install_handler(1, test);
	idt_init();
	for(;;);
}

