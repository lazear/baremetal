/*
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

/*
http://www.intel.com/design/pentium/datashts/24201606.pdf

BSP: Bootstrap processor - the core we are currently running on
AP: Application processor(s). Currently halted.
*/
#include <types.h>
#include <smp.h>
#include <acpi.h>
#include <lapic.h>
#include <traps.h>
#include <x86.h>
#include <mutex.h>

#define INIT       0x00000500   // INIT/RESET
#define STARTUP    0x00000600   // Startup IPI
#define DELIVS     0x00001000   // Delivery status
#define ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define DEASSERT   0x00000000
#define LEVEL      0x00008000   // Level triggered
#define BCAST      0x00080000   // Send to all APICs, including self.
#define BUSY       0x00001000
#define FIXED      0x00000000

volatile uint32_t* LAPIC = NULL;




static void lapic_write(int index, int value) {
	LAPIC[index/4] = value;		// div by 4 for sizeof(int)
	LAPIC[0x8];					// Wait by reading;
	//dprintf("[lapic] write %x to %x\n", value, index);	
}

static uint32_t lapic_read(int index) {
	return LAPIC[index/4];
}

void lapic_test() {
	lapic_write(LAPIC_ICRLO, (3<<18) | (1<<11) | (1<<14) | (0<<8) | 0x25);
}

void lapic_eoi() {
	if (!LAPIC)
		return;
	lapic_write(LAPIC_EOI, 0);
}

/* Initilize the local advanced programmable interrupt chip */
void lapic_init() {

	/* Use default Local APIC location */
	LAPIC = LAPIC_BASE;

	/* Paging is enabled, so we need to map in the physical mem */
	if (k_phys_to_virt(LAPIC_BASE))
		dprintf("[lapic] non-fatal error: LAPIC_BASE already mapped\n");
	k_paging_map(LAPIC_BASE, LAPIC_BASE, 0x3);


//	pic_disable();
	/* Enable local APIC and set the spurious interrupt vector */
	dprintf("LAPIC_SIV %x\n", lapic_read(LAPIC_SIV));
	//lapic_write(LAPIC_SIV, 0x100 | (IRQ0 + IRQ_SPURIOUS));
	lapic_write(LAPIC_SIV, 0x1ff);
	dprintf("LAPIC_SIV %x\n", lapic_read(LAPIC_SIV));
	// Calibrate timer
	lapic_write(LAPIC_TIMER, (1<<17) | 0x20);
	//lapic_write(LAPIC_TDCR, 0x00020000 | 0x20);	// Irq0 + IRQ_timer | Periodic
	lapic_write(0x380, 0xF0000000);

	dprintf("CC: %d\n", lapic_read(0x390));
	/* Mask interrupts */
	//lapic_write(LAPIC_LINT0, 0x10000);
	//lapic_write(LAPIC_LINT1, 0x10000);

	if ((lapic_read(LAPIC_VER) >> 16) & 0xFF >= 4)
		lapic_write(0x0340, 0x10000);

	lapic_write(LAPIC_ERR, IRQ0 + IRQ_ERROR);
	lapic_write(LAPIC_ERR, 0);
	lapic_write(LAPIC_EOI, 0);	// Clear any existing interrupts

	lapic_write(LAPIC_ICRHI, 0);
	lapic_write(LAPIC_ICRLO, INIT | LEVEL | BCAST);


	while(LAPIC[LAPIC_ICRLO] & DELIVS)
		;
	dprintf("CC: %x\n", lapic_read(0x390));
	lapic_write(LAPIC_TPR, 0);
dprintf("CC: %x\n", lapic_read(0x390));
}


mutex proc_m = {.lock = 0};

int udelay(int i) {
	lapic_read(LAPIC_ID);
}

/* Sends a start up IPI to the AP processor designated <apic_id>,
telling it to start executing code at <address> 
Address must be page-aligned, and in the first megabyte of system mem*/
void lapic_start_AP(int apic_id, uint32_t address) {

	/* Following Intel SMP startup procedure:
	Write 0Ah to CMOS RAM location 0Fh (shutdown code). 
	This initializes a warm reset
	*/
	outb(0x70, 0xF);
	outb(0x71, 0xA);

	/*
	The STARTUP IPI causes the target processor to start executing in Real Mode from address
	000VV000h, where VV is an 8-bit vector that is part of the IPI message. Startup vectors are
	limited to a 4-kilobyte page boundary in the first megabyte of the address space. Vectors A0-BF
	are reserved; do not use vectors in this range. 
	*/
	uint16_t* warm_reset_vector = (uint16_t) P2V(0x40 << 4 | 0x67);
	warm_reset_vector[0] = 0;
	warm_reset_vector[1] = address >> 4;
	//*warm_reset_vector = address;

	lapic_write(LAPIC_ICRHI, apic_id << 24);
	lapic_write(LAPIC_ICRLO, INIT | LEVEL | ASSERT);
	udelay(2);
	lapic_write(LAPIC_ICRLO, INIT | LEVEL);
	udelay(1);

	/* Keep this printf here, it acts as a delay... lol */
	dprintf("[lapic] starting cpu: %d\n", apic_id);
	for (int i = 0; i < 2; i++) {
		lapic_write(LAPIC_ICRHI, apic_id << 24);
		lapic_write(LAPIC_ICRLO, STARTUP |  address >> 12);
		udelay(1);
	}

}


extern uint32_t _binary_ap_entry_start[];
extern uint32_t _binary_ap_entry_end[];
extern uint32_t KERNEL_PAGE_DIRECTORY;



static inline void lidt(struct gatedesc *p, int size) {
	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void lgdt(struct segdesc *p, int size) {

	// 16:32 bit pointer is required for lgdt/lidt
	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;
	asm volatile("lgdt (%0)" : : "r" (pd));
}


volatile int ncpu = 0;


void mp_enter() {
	acquire(&proc_m);
	int id = lapic_read(LAPIC_ID) >> 24;
//	ncpu = (ncpu > id) ? ncpu : id;
	ncpu++;

	// Initialize CPU-local GDT and global IDT 
	gdt_init_cpu(id);
	lidt(idt, sizeof(idt));
		cpu->id = id;
	release(&proc_m);

	scheduler();

}

int mp_processor_id() {
	//if interrupts enabled, panic
	if (get_eflags() & 0x200) {
		panic("INTERRUPTS ENABLED");
		return -1;
	}
	return lapic_read(LAPIC_ID) >> 24;
}


int mp_number_of_processors() {
	return ncpu + 1;	
}


#define STACK_SIZE	0x4000
void mp_start_ap(int nproc) {
	int ap_entry_size = (uint32_t) _binary_ap_entry_end - (uint32_t) _binary_ap_entry_start;
	uint8_t* code = 0x8000;
	memmove(code, _binary_ap_entry_start, ap_entry_size);

	for (int i = 1; i < nproc; i++) {

		//pushcli();
		uint32_t stack = (malloc(STACK_SIZE) + STACK_SIZE) & ~0xF;
		cpus[i].stack = stack;
		cpus[i].id = i;
		cpus[i].cpu = &cpus[i];
		// Pass several arguments to the ap-start up code
		*(void**)(code - 4) = stack;
		*(void**)(code - 8) = mp_enter;
		*(int**)(code - 12) = V2P(KERNEL_PAGE_DIRECTORY);
		
		lapic_start_AP(i, 0x8000);
		//popcli();
	}

}