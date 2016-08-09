/*
Michael Lazear, (C) 2007-2016

kernel.c
*/

#include <kernel.h>
#include <vga.h>

void kernel_initialize() {

	vga_setcolor(0x2);
	vga_clear();

	vga_puts("Hello, World from baremetal!\n");
	vga_puts("Newline test");
	for(;;);
}