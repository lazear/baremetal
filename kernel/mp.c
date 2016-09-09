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

struct mp {             // floating pointer
  uint8_t signature[4];           // "_MP_"
  void *physaddr;               // phys addr of MP config table
  uint8_t length;                 // 1
  uint8_t specrev;                // [14]
  uint8_t checksum;               // all bytes must add up to 0
  uint8_t type;                   // MP system config type
  uint8_t imcrp;
  uint8_t reserved[3];
};

struct mpconf {         // configuration table header
  uint8_t signature[4];           // "PCMP"
  uint16_t length;                // total table length
  uint8_t version;                // [14]
  uint8_t checksum;               // all bytes must add up to 0
  uint8_t product[20];            // product id
  uint32_t *oemtable;               // OEM table pointer
  uint16_t oemlength;             // OEM table length
  uint16_t entry;                 // entry count
  uint32_t *lapicaddr;              // address of local APIC
  uint16_t xlength;               // extended table length
  uint8_t xchecksum;              // extended table checksum
  uint8_t reserved;
};

struct mpproc {         // processor table entry
  uint8_t type;                   // entry type (0)
  uint8_t apicid;                 // local APIC id
  uint8_t version;                // local APIC verison
  uint8_t flags;                  // CPU flags
    #define MPBOOT 0x02           // This proc is the bootstrap processor.
  uint8_t signature[4];           // CPU signature
  uint32_t feature;                 // feature flags from CPUID instruction
  uint8_t reserved[8];
};

struct mpioapic {       // I/O APIC table entry
  uint8_t type;                   // entry type (2)
  uint8_t apicno;                 // I/O APIC id
  uint8_t version;                // I/O APIC version
  uint8_t flags;                  // I/O APIC flags
  uint32_t *addr;                  // I/O APIC address
};

struct cpu {
  uint8_t id;                    // Local APIC ID; index into cpus[] below
  //struct context *scheduler;   // swtch() here to enter scheduler
  //struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[8];   // x86 global descriptor table
  volatile uint32_t started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  
  // Cpu-local storage variables; see below
  struct cpu *cpu;
  //struct proc *proc;           // The currently-running process.
} cpus[8];

// Table entry types
#define MPPROC    0x00  // One per processor
#define MPBUS     0x01  // One per bus
#define MPIOAPIC  0x02  // One per I/O APIC
#define MPIOINTR  0x03  // One per bus interrupt source
#define MPLINTR   0x04  // One per system interrupt source
