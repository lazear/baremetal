/*
task.c
*/

#include <types.h>
#include <x86.h>

/*
alloc a new 4kb stack for the thread
*/
uint32_t* k_task_generate_stack(void (*fn)()) {

	uint32_t* stack = malloc(0x1000) + 0x1000;
	kprintx("New stack@ ", stack);
	// processor data (iret)

	*--stack = 0x200;	// EFLAGS
	*--stack = 0x08;	// CS
	*--stack = (uint32_t) fn;	// EIP
	// pusha
	*--stack = 0;		// EDI
	*--stack = 0;		// ESI

	*--stack = 0;		// EBP
	*--stack = (uint32_t) stack;
	*--stack = 0;		// EBX
	*--stack = 0;		// EDX
	*--stack = 0;		// ECX
	*--stack = 0;		// EAX

	// data segments
	*--stack = 0x10;	// DS
	*--stack = 0x10;	// ES
	*--stack = 0x10;	// FS
	*--stack = 0x10;	// GS
	return stack;
}

regs_t* k_new_stack(void (*fn)()) {

	uint32_t* stack = malloc(0x1000) + 0x1000;
	regs_t* r = malloc(sizeof(regs_t));

	*--stack = 0x200;	// EFLAGS
	*--stack = 0x08;	// CS
	*--stack = (uint32_t) fn;	// EIP
	// pusha
	*--stack = 0;		// EDI
	*--stack = 0;		// ESI

	*--stack = 0;		// EBP
	*--stack = (uint32_t) stack;
	*--stack = 0;		// EBX
	*--stack = 0;		// EDX
	*--stack = 0;		// ECX
	*--stack = 0;		// EAX

	// data segments
	*--stack = 0x10;	// DS
	*--stack = 0x10;	// ES
	*--stack = 0x10;	// FS
	*--stack = 0x10;	// GS

	r->esp = (uint32_t) stack;
	r->eip = (uint32_t) fn;
	r->flags = 0x200;
	r->cs = 0x08;
	r->ds = 0x10;
	r->es = 0x10;
	r->fs = 0x10;
	r->gs = 0x10;


	//kprintx("New stack@ ", stack);
	return r->esp;
}
