/*
sched.c
*/

#include <types.h>
#include <x86.h>
#include <threads.h>


uint32_t* make_stack(void (*fn)() ) {
	cli();
	uint32_t* stack = malloc(0x1000) + 0x1000; // point to top of stack
	uint32_t* top = (uint32_t) stack;

	*--stack = 0x202;	// EFLAGS
	*--stack = 0x10;
	*--stack = (uint32_t) fn;	// EIP
	// pusha
	*--stack = 0;		// EAX
	*--stack = 0;		// ECX
	*--stack = 0;		// EDX
	*--stack = 0;		// EBX
	*--stack = top;		// ESP
	*--stack = top;		// EBP
	*--stack = 0;		// ESI
	*--stack = 0;		// EDI
	// data segments
	*--stack = 0x10;	// DS
	*--stack = 0x10;	// ES
	*--stack = 0x10;	// FS
	*--stack = 0x10;	// GS
	//t->esp = stack;

	//printf("Thread %s created, with PID= %d\n", t->name, t->pid);
	sti();
	return stack;
}

typedef struct context_t {
//	uint32_t ecx, edx, eax;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} __attribute__((packed)) context;

extern uint32_t* get_context();

context* proc_spawn(void (*fn)() ) {
	//cli();
	uint32_t* stack = (uint32_t*) (malloc(0x1000));
	uint32_t top = ((uint32_t) stack + 0x1000);
	memset(stack, 0, 0x1000);

	stack = top;

	*--stack = (uint32_t) fn;
	*--stack = top;
	*--stack = 0x9500; //t->ebx;
	*--stack = 0; //t->esi;
	*--stack = 0; //t->edi;
	*--stack = 0x202;

	uint32_t* ptr = stack;

	for (int i = 0; i < 5; i++)
		printf("(frame %d) %x\n", i,*ptr++);
	printf("Stack top: %x\n Bottom: %x\n", top, stack);
	//sti();
		//blockinfo(find_block(stack));
	return stack;
}

extern void switch_context(context* a, context* b);



int iq = 0;

void fn1() {
	printf("FN1\n");
	printf("Hello FN1 %d\n", iq++);
	for(;;);
}

void fn2() {
	printf("FN2\n");
	printf("Hello FN2\n");
	sched_asm();
	printf("WE MADE IT");
	for(;;);
}




uint32_t slock = 0;
	context* a = 0;
	context* b = 0;

uint32_t* get_context() {
	uint32_t* esp;
	asm volatile("push %ebp");
	asm volatile("push %ebx");
	asm volatile("push %esi");
	asm volatile("push %edi");

	asm volatile("movl %%esp, %0" : "=r" (esp));

	printf("esp from context %x\n", esp);

	asm volatile ("pop %edi; pop %esi; pop %ebx; pop %ebp");

	//context* c = (context*) esp;
	// printf("eip %x\n", c->eip);
	// printf("ebp %x\n", c->ebp);
	// printf("ebx %x\n", c->ebx);
	// printf("esi %x\n", c->esi);
	// printf("edi %x\n", c->edi);
	return esp;
}


void set_context(uint32_t* esp) {
	asm volatile ( "movl %0, %%esp; \
					pop %%edi; \
					pop %%esi; \
					pop %%ebx; \
					pop %%ebp; " : : "r"(esp));
}

int running = 0;
uint32_t* current = 0;
uint32_t* saved = 0;


uint32_t swap(uint32_t* esp) {
	if (saved == esp) {
		saved = current;
		current = esp;
	} else {
		saved = esp;
	}
	return saved;
}

void sched(uint32_t esp) {
	//cli();
	printf("%x\n", esp);
	//set_eflags(0x202);
	return swap(esp);
	//return esp;
}

extern void sched_handler();

void sched_init() {

	current = proc_spawn(fn2);
	printf("Made stack @ %x\n", current);
	sched_asm();


//	sched_asm();
//	saved = get_context();
//	idt_set_gate(32, (uint32_t) sched_handler, 0x08, 0x8E);
}