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

typedef struct  {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t rev;
	uint32_t rsdt_ptr;		
	uint32_t length;
	uint64_t* xsdt_ptr;
	uint8_t extchecksum;
	uint8_t pad[3];
} rsdp_header __attribute__((packed));


typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t rev;
	uint8_t checksum;
	char oemid[6];
	char oemtableid[8];
	uint32_t oemrev;
	uint32_t creatorid;
	uint32_t creatorrev;
} acpi_header __attribute__((packed));

typedef struct {
	acpi_header h;
	uint32_t tableptrs[];
} rsdt_header __attribute__((packed));


typedef struct {
	uint8_t type;			// ==0;
	uint8_t rec_len;
	uint8_t acpi_proc_id;
	uint8_t apic_id;
	uint32_t flags;
} acpi_lapic;

typedef struct {
	uint8_t type;			// ==1
	uint8_t rec_len;
	uint8_t ioapic_id;
	uint8_t res;
	uint32_t ioapic_addr;
	uint32_t gsib;			// Global System Interrupt Bus
} acpi_ioapic;

typedef struct {
	uint8_t type;		// ==2
	uint8_t rec_len;	// Record len;
	uint8_t bus_src;	// Bus source
	uint8_t irq_src;	// IRQ source
	uint8_t gsi;		// Global System Interrupt
	uint16_t flags;
} acpi_iso;				// Int. source override

typedef struct  {
	acpi_header h;
	uint32_t lca;		// local controller address;
	uint32_t flags;
	// union {
	// 	acpi_iso iso;
	// 	acpi_ioapic ioapic;
	// 	acpi_lapic lapic;
	// } entries[];
	uint8_t entries[];
} madt_header;

int acpi_checksum(char* ptr) {
	int sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += ptr[i];
	return (char) sum;

}

void acpi_parse_madt(madt_header* madt) {
	printf("%s\tflags %d\n", madt->h.signature, (madt->h.length - sizeof(madt_header)));
	int type = 0;
	int i = 0;
	do {		
		int type = madt->entries[i];
		int len = madt->entries[i+1];

		printf("type: %d len %d\n", type, len);
		switch(type) {
			case 0: {
				acpi_lapic *x = (acpi_lapic*) (uint32_t) madt->entries + i;
				printf("LAPIC: processor id %d %d %d\n", x->acpi_proc_id, x->apic_id, x->flags);
				break;
			}
			case 1: {
				acpi_ioapic *x = (acpi_ioapic*) (uint32_t) madt->entries + i;
				//printf("LAPIC: processor id %d %d\n", x->ioapic_id, x->ioapic_addr);
				break;
			}
			case 2: {
				acpi_iso *x = (acpi_iso*) (uint32_t) madt->entries + i;
				//printf("LAPIC: processor id %d %d\n", x->ioapic_id, x->ioapic_addr);
				break;
			}
			default:
				return;
		}
				i += len;
	} while(type < 3);


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