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

void acpi_parse_madt(madt_header* madt) {
	char* entries = (char*) ((uint32_t) madt + sizeof(madt_header));
	while( (uint32_t) entries < (madt->h.length - sizeof(madt_header)) + (uint32_t)madt) {		
		int type = *entries;
		int len = *(entries+1);
		switch(type) {
			case 0: {
				acpi_lapic *x = (acpi_lapic*) entries;// + i;
				printf("LAPIC: processor id %d\tapic id%d\tflags:%d\n", x->acpi_proc_id, x->apic_id, x->flags);
				break;
			}
			case 1: {
				acpi_ioapic *x = (acpi_ioapic*) entries;// + i;
				printf("I/O APIC: id %d\taddress:0x%x\n", x->ioapic_id, x->ioapic_addr);
				break;
			}
			case 2: {
				acpi_iso *x = (acpi_iso*)  entries;// + i;
				printf("IRQ src %d BUS src %d GSI %d\n", x->irq_src, x->bus_src, x->gsi);
				break;
			}
		}
		entries += len;
	} 
} 


void acpi_init() {

    uint8_t *ptr = (uint8_t *)0x000E0000;
	while (ptr < 0x000FFFFF) {
		uint64_t signature = *(uint64_t *)ptr;
		if (signature == 0x2052545020445352) { // 'RSD PTR '
			printf("FOUND THE ACPI BITCH\n");
			break;
		}
		ptr += 16;
	}
	rsdp_header* h = (rsdp_header*) ptr;
	printf("ACPI OEM: %s\n", h->oemid);

	assert(acpi_checksum(h) == 0);

	rsdt_header* r = (rsdt_header*) h->rsdt_ptr; // + 0xC0000000;

	k_paging_map(h->rsdt_ptr, P2V(h->rsdt_ptr), 0x7);
	r = P2V(r);
	printf("RSDT: %s\t# of tables: %d\n", r->h.signature, (r->h.length - sizeof(acpi_header)) /4 );

	for (int i = 0; i < (r->h.length - sizeof(acpi_header)) /4; i++) {
	//	printf("Table[%d]: 0x%x\n", i, r->tableptrs[i]);
		acpi_header* entry = (acpi_header*) P2V(r->tableptrs[i]);
		k_paging_map(r->tableptrs[i], P2V(r->tableptrs[i]), 0x7);
		if (!strncmp(entry->signature, "APIC", 4)) {
			printf("FOUND APIC FINALLY THANK FUCKING GOD\n");
			acpi_parse_madt(entry);
			break;
		}
	}

}