/*
ioapic.c
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
I/O APIC manages system-wide IO interrupt events on a SMP system
http://www.intel.com/design/chipsets/datashts/29056601.pdf
*/

#include <types.h>
#include <traps.h>

#define IOAPIC  	0xFEC00000  /* Default physical address of IO APIC */
#define IOREGSEL	0x00 		/* This register selects the IOAPIC register to be read/written */
#define IOWIN		0x10 		/* This register is used to read and write from the reg selected by IOREGSEL */


#define IOAPICID 	0x00 		/* Identification register - 4 bit APIC ID */
#define IOAPICVER	0x01 
#define IOREDTBL 	0x10 		/* Base address of IO Redirection tables */


static uint32_t ioapic_read(int reg) {
	*(uint32_t*)(IOAPIC + IOREGSEL) = reg;
	return *(uint32_t*)(IOAPIC + IOWIN);
}

static void ioapic_write(int reg, uint32_t data) {
	*(uint32_t*)(IOAPIC + IOREGSEL) = reg;
	*(uint32_t*)(IOAPIC + IOWIN) = data;
}

void ioapic_enable(uint8_t irq, uint16_t cpu) {
	
	/* Write the low 32 bits :
	31:17 reserved
	16: Interrupt mask:		1 = masked. 0 = enabled
	15: Trigger mode:		1 = Level sensitive. 0 = Edge sensitive
	14: Remote IRR:			1 = LAPIC accept. 0 = LAPIC sent EOI, and IOAPIC received
	13: INTPOL: 			1 = Low active polarity. 0 = High active polarity
	12: Delivery Stat:		1 = Send Pending. 0 = IDLE
	11: Destination Mode:	1 = Logical Mode (Set of processors.. LAPIC id?). 0 = Physical mode, APIC ID
	10:8 Delivery Mode:
		000 Fixed
		001 Lowest priority
		010 SMI: System Management Interrupt. Requires edge trigger mode
		011 Reserved
		100 NMI
		101 INIT
		110 Reserved
		111 ExtINT
	7:0 Interrupt Vector: 8 bit field containing the interrupt vector, from 0x10 to 0xFE
	*/
	uint32_t low = (IRQ0 + irq);
	dprintf("[ioapic] Enabling irg %d on cpu %d (low dword: %#x)\n", irq, cpu, low);
	ioapic_write(IOREDTBL + (irq * 2), low );
	/* Write the high 32 bites. 63:56 contains destination field */
	ioapic_write(IOREDTBL + (irq * 2) + 1, cpu << 24);
}

void ioapic_disable(uint8_t irq) {
	ioapic_write(IOREDTBL + (irq*2), (1<<16) | (0x20 + irq));
	ioapic_write(IOREDTBL + (irq*2) +1, 0);
}

void ioapic_init(){
	if (k_phys_to_virt(IOAPIC))
		dprintf("[ioapic] IOAPIC_BASE is already mapped\n");
	else
		k_paging_map(IOAPIC, IOAPIC, 0x3);

	uint32_t ver = ioapic_read(0x01);
	uint16_t max = (ver >> 16) & 0xFF;
	dprintf("[ioapic] version %#x max reg table entries: %d\n", ver, max);
	if (ver != 0x00170011) 
		panic("Faulty IOAPIC version!\n");
	for (int i = 0; i < max; i++)
		ioapic_disable(i);
}



