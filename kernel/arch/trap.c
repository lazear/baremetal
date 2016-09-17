/*
trap.c
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
#include <types.h>
#include <x86.h>
#include <traps.h>

/* Should switch back to using an array of handlers */
void trap(regs_t* r) {
	switch (r->int_no) {
		case 14:
			k_page_fault(r);
			break;
		case IRQ0 + IRQ_TIMER:
			timer(r);
			break;
		case IRQ0 + IRQ_KBD:
			keyboard_handler(r);
			break;
		case IRQ0 + IRQ_IDE:
			ide_handler();
			break;
		case 0x80:
			syscall(r);
			break;
	}

	if (r->int_no > 40)
		outb(0xA0, 0x20);
	
	outb(0x20, 0x20);

	lapic_eoi();
}
