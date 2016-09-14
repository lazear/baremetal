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

#ifndef __crunchy_acpi__
#define __crunchy_acpi__

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
} acpi_lapic __attribute__((packed));

typedef struct {
	uint8_t type;			// ==1
	uint8_t rec_len;
	uint8_t ioapic_id;
	uint8_t res;
	uint32_t ioapic_addr;
	uint32_t gsib;			// Global System Interrupt Bus
} acpi_ioapic __attribute__((packed));

typedef struct {
	uint8_t type;		// ==2
	uint8_t rec_len;	// Record len;
	uint8_t bus_src;	// Bus source
	uint8_t irq_src;	// IRQ source
	uint8_t gsi;		// Global System Interrupt
	uint16_t flags;
} acpi_iso __attribute__((packed));				// Int. source override

typedef struct  {
	acpi_header h;
	uint32_t lca;		// local controller address;
	uint32_t flags;
} madt_header __attribute__((packed));

#endif