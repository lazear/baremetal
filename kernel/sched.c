/*
sched.c
*/

#include <types.h>
#include <x86.h>
#include <threads.h>


typedef struct context_t {
//	uint32_t ecx, edx, eax;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} __attribute__((packed)) context;



context* proc_spawn(void (*fn)() ) {
	//cli();
	uint32_t* stack = (uint32_t*) malloc(sizeof(context));
	memset(stack, 0, sizeof(context));

//	uint32_t* test = (uint32_t*) malloc(0x1000) + 0x1000;
	//uint32_t* ts = (uint32_t) test

	context* t = (context*) stack;
	t->eip = (uint32_t) fn;
	t->ebp = (uint32_t) stack;				// point to highest address of stack
	t->ebx = 0;
	t->esi = 0;
	t->edi = 0;
	//sti();
	printf("malloc %x\n", stack);
	//blockinfo(find_block(stack));
	return t;
}

extern void switch_context(context* a, context* b);
extern uint32_t get_context(context* current);


void pwait(int n) {
	int waitfor = get_ticks() + n;
	while (waitfor > get_ticks) sched();
}
int iq = 0;

void fn1() {
	printf("FN1\n");
	printf("Hello FN1 %d\n", iq++);
	
	sched();
	return;
}

void fn2() {
	printf("FN2\n");
	printf("Hello FN2\n");
	sched();
	return;
}

int intena = 0;


void acquire(uint32_t* lock) {
	while(*lock);
	*lock = 1;
	printf("%x locked\n", lock);
}

void release(uint32_t* lock) {
	*lock = 0;
	printf("%x released\n", lock);
}

int holding(uint32_t* lock) {
	return *lock;
}

uint32_t slock = 0;
	context* a = 0;
	context* b = 0;
void sched(void) {
	//cli();
	if (!holding(&slock))
		acquire(&slock);

	printf("sched\n");
	if (intena) {
		intena = 0;
		release(&slock);
		switch_context(b, a);
		printf("loop0\n");
	
	} else {
		intena = 1;
		release(&slock);
		switch_context(a, b);	
		printf("loop1\n");	
	}

	//sti();
}

void sched_init() {

	a = proc_spawn(fn1);
	b = proc_spawn(fn2);
	context* c = malloc(sizeof(context));
	context* esp = get_context(c);
	uint32_t *i = (uint32_t) esp;
	printf("get-context %x, %x\n", esp, c);
	for (int q = 0; q < sizeof(context)/2; q++)
		printf("(%d frame) %x\n", q, *i++);

	i = (uint32_t) esp;
	printf("%x\n", esp->eip);

	printf("%x\n", esp->ebp);
	printf("%x\n", esp->ebx);
	printf("%x\n", esp->esi);
	printf("%x\n", esp->edi);

}