/*
mp.c
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
#include <assert.h>
#include <acpi.h>


int acpi_checksum(char* ptr) {
	int sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += ptr[i];
	return (char) sum;

}

int acpi_parse_madt(madt_header* madt) {
	char* entries = (char*) ((uint32_t) madt + sizeof(madt_header));
	int num_lapics = 0;
	while( (uint32_t) entries < (madt->h.length - sizeof(madt_header)) + (uint32_t)madt) {		
		int type = *entries;
		int len = *(entries+1);
		switch(type) {
			case 0: {
				acpi_lapic *x = (acpi_lapic*) entries;// + i;
				dprintf("[acpi] lapic: processor id %d\tapic id%d\tflags:%d\n", x->acpi_proc_id, x->apic_id, x->flags);
				num_lapics++;
				break;
			}
			case 1: {
				acpi_ioapic *x = (acpi_ioapic*) entries;// + i;
				dprintf("[acpi] ioapic: id %d\taddress:0x%x\n", x->ioapic_id, x->ioapic_addr);
				break;
			}
			case 2: {
				acpi_iso *x = (acpi_iso*)  entries;// + i;
				dprintf("[acpi] irq src %d bus src %d gsi %d\n", x->irq_src, x->bus_src, x->gsi);
				break;
			}
		}
		entries += len;
	} 
	return num_lapics;
} 

int acpi_init() {
	/* Read through the extended bios data area (EBDA) and look at every 16-byte aligned
	structure for the signature */
    uint8_t *ptr = (uint8_t *) P2V(0x000E0000);

	while (ptr < P2V(0x000FFFFF)) {
		uint64_t signature = *(uint64_t *)ptr;
		if (signature == 0x2052545020445352) { // 'RSD PTR '
			break;
		}
		ptr += 16;
	}


	rsdp_header* h = (rsdp_header*) ptr;

	/* Make sure we have a valid ACPI table. lower byte of the sum of the first 20 bytes
	(isn't that a mouthful?) need to sum to zero */
	assert(acpi_checksum(h) == 0);
	if (acpi_checksum(h))
		return;

	rsdt_header* r = (rsdt_header*) h->rsdt_ptr; // + 0xC0000000;

	k_paging_map(h->rsdt_ptr, P2V(h->rsdt_ptr), 0x7);
	r = P2V(r);
	for (int i = 0; i < (r->h.length - sizeof(acpi_header)) /4; i++) {
		acpi_header* entry = (acpi_header*) P2V(r->tableptrs[i]);
		k_paging_map(r->tableptrs[i], P2V(r->tableptrs[i]), 0x7);
		if (!strncmp(entry->signature, "APIC", 4)) {
			return acpi_parse_madt(entry);
		}
	}
	panic("No APIC table found. SYSTEM FAILURE\n");
	k_paging_unmap(r);

}