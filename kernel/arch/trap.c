
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
	}

	if (r->int_no > 40)
		outb(0xA0, 0x20);
	
	outb(0x20, 0x20);
}
