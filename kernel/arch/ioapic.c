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

#define IOAPIC  0xFEC00000   // Default physical address of IO APIC

#define REG_ID     0x00  // Register index: ID
#define REG_VER    0x01  // Register index: version
#define REG_TABLE  0x10  // Redirection table base

// The redirection table starts at REG_TABLE and uses
// two registers to configure each interrupt.  
// The first (low) register in a pair contains configuration bits.
// The second (high) register contains a bitmask telling which
// CPUs can serve that interrupt.
#define INT_DISABLED   0x00010000  // Interrupt disabled
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)

volatile struct ioapic *ioapic;

// IO APIC MMIO structure: write reg, then read or write data.
struct ioapic {
uint32_t reg;
uint32_t pad[3];
uint32_t data;
};

static uint32_t
ioapicread(int reg)
{
ioapic->reg = reg;
return ioapic->data;
}

static void
ioapicwrite(int reg, uint32_t data)
{
ioapic->reg = reg;
ioapic->data = data;
}

void
ioapicinit(void) {
	int i, id, maxintr;

	if (k_phys_to_virt(IOAPIC))
	vga_pretty("LAPIC_BASE is already mapped\n", 4);
	else
	k_paging_map(IOAPIC, IOAPIC, 0x3);

	ioapic = (volatile struct ioapic*) IOAPIC;
	maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
	id = ioapicread(REG_ID) >> 24;
	// if(id != ioapicid)
	//   cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");

	// Mark all interrupts edge-triggered, active high, disabled,
	// and not routed to any CPUs.
	for(i = 0; i <= maxintr; i++){
		ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (IRQ0 + i));
		ioapicwrite(REG_TABLE+2*i+1, 0);
	}
}

void
ioapicenable(int irq, int cpunum)
{
// Mark interrupt edge-triggered, active high,
// enabled, and routed to the given cpunum,
// which happens to be that cpu's APIC ID.
ioapicwrite(REG_TABLE+2*irq, IRQ0 + irq);
ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
