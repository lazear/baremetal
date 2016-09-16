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
BSP: Bootstrap processor - the core we are currently running on\
AP: Application processor(s). Currently halted.
*/
#include <types.h>
#include <acpi.h>
#include <lapic.h>
#include <traps.h>
#include <x86.h>

volatile uint32_t* LAPIC;

static void lapic_write(int index, int value) {
	LAPIC[index/4] = value;		// div by 4 for sizeof(int)
	LAPIC[0x8];					// Wait by reading;
}

static uint32_t lapic_read(int index) {
	return LAPIC[index/4];
}

/* Initilize the local advanced programmable interrupt chip */
void lapic_init() {
	LAPIC = LAPIC_BASE;
	if (k_phys_to_virt(LAPIC_BASE))
		vga_pretty("LAPIC_BASE is already mapped\n", 4);
	k_paging_map(LAPIC_BASE, LAPIC_BASE, 0x3);
	printf("LAPIC id: %d\n", lapic_read(LAPIC_ID) >> 24);

	/* Enable local APIC and set the spurious interrupt vector */
	lapic_write(LAPIC_SIV, 0x100 | (IRQ0 + IRQ_SPURIOUS));

	
}

/* Sends a start up IPI to the AP processor designated <apic_id>,
telling it to start executing code at <address> */
void lapic_start_AP(int apic_id, uint32_t address) {

}
