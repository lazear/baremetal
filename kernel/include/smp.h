
#ifndef __smp__
#define __smp__

#include <types.h>
#include <x86.h>

struct cpu {
	uint32_t id;
	uint32_t stack;
	int ncli;
	struct segdesc gdt[8] __attribute__((aligned(32)));
	struct cpu* cpu;		/* CPU local storage */
} cpus[8];

extern struct cpu* cpu asm("%gs:0");


#endif