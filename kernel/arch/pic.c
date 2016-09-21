/*
pic.c
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
Programmable Interrupt Controller
*/

#include <types.h>
#include <traps.h>


#define PIC1_CMD		0x20
#define PIC1_DATA		0x21
#define PIC2_CMD		0xA0
#define PIC2_DATA		0xA1

#define IRQ_SLAVE       2       // IRQ at which slave connects to master

// Current IRQ mask.
// Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
static uint16_t irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);

/* Mask interrupts on the PIC */
static void pic_setmask(uint16_t mask) {
	irqmask = mask;
	outb(PIC1_DATA, mask);
	outb(PIC2_DATA, mask >> 8);
}

void pic_enable(int irq) {
 	pic_setmask(irqmask & ~(1<<irq));
}

/* Mask all interrupts */
void pic_disable() {
	pic_setmask(0xFFFF);
}

// Initialize the 8259A interrupt controllers.
void pic_init(void) {
	// outb(0x20, 0x11);
	// outb(0xA0, 0x11);
	// outb(0x21, 0x20);
	// outb(0xA1, 0x28);
	// outb(0x21, 0x04);
	// outb(0xA1, 0x02);
	// outb(0x21, 0x01);
	// outb(0xA1, 0x01);
	// pic_setmask(irqmask);

	//outb(0x21, 0x0);
	//outb(0xA1, 0x0);
	// mask all interrupts
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	// Set up master (8259A-1)

	// ICW1:  0001g0hi
	//    g:  0 = edge triggering, 1 = level triggering
	//    h:  0 = cascaded PICs, 1 = master only
	//    i:  0 = no ICW4, 1 = ICW4 required
	outb(PIC1_CMD, 0x11);

	// ICW2:  Vector offset
	outb(PIC1_DATA, IRQ0);

	// ICW3:  (master PIC) bit mask of IR lines connected to slaves
	//        (slave PIC) 3-bit # of slave's connection to master
	outb(PIC1_DATA, 1<<IRQ_SLAVE);

	// ICW4:  000nbmap
	//    n:  1 = special fully nested mode
	//    b:  1 = buffered mode
	//    m:  0 = slave PIC, 1 = master PIC
	//      (ignored when b is 0, as the master/slave role
	//      can be hardwired).
	//    a:  1 = Automatic EOI mode
	//    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
	outb(PIC1_DATA, 0x3);

	// Set up slave (8259A-2)
	outb(PIC2_CMD, 0x11);                  // ICW1
	outb(PIC2_DATA, IRQ0 + 8);            // ICW2
	outb(PIC2_DATA, IRQ_SLAVE);           // ICW3
	// NB Automatic EOI mode doesn't tend to work on the slave.
	// Linux source code says it's "to be investigated".
	outb(PIC2_DATA, 0x3);                 // ICW4

	// OCW3:  0ef01prs
	//   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
	//    p:  0 = no polling, 1 = polling mode
	//   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
	outb(PIC1_CMD, 0x68);             // clear specific mask
	outb(PIC1_CMD, 0x0a);             // read IRR by default

	outb(PIC2_CMD, 0x68);             // OCW3
	outb(PIC2_CMD, 0x0a);             // OCW3

	if(irqmask != 0xFFFF)
		pic_setmask(irqmask);
}
