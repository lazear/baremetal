/*
task.c
*/

#include <types.h>
#include <x86.h>

typedef struct THREAD {
	struct THREAD* previous;
	char* name;
	uint32_t pid;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eip;
	uint32_t cr3;
	uint32_t state;

	struct THREAD* next;

} thread;


thread *k_thread = 0;

thread* current_thread = 0;

uint32_t K_SCHED_ACTIVE = 0;
uint32_t K_THREAD_COUNT = 0;

thread* k_create_thread(char* name, void (*fn)() ) {
	uint32_t* stack = malloc(0x1000) + 0x1000; // point to top of stack

	thread* t = (thread*) malloc(sizeof(thread));
	memset(t, 0, sizeof(thread));

	t->name = name;
	t->pid = K_THREAD_COUNT++;
	t->eip = (uint32_t) fn;
	t->esp = (uint32_t) stack - 0x1000;
	t->ebp = (uint32_t) stack;		// point to bottom of stack
	t->state = 1;


	*--stack = 0x202;	// EFLAGS
	*--stack = 0x08;	// CS
	*--stack = (uint32_t) fn;	// EIP
	// pusha

	*--stack = 0;		// EAX
	*--stack = 0;		// ECX
	*--stack = 0;		// EDX
	*--stack = 0;		// EBX
	*--stack = t->esp;		// ESP
	*--stack = t->ebp;		// EBP

	*--stack = 0;		// ESI
	*--stack = 0;		// EDI
	// data segments
	*--stack = 0x10;	// DS
	*--stack = 0x10;	// ES
	*--stack = 0x10;	// FS
	*--stack = 0x10;	// GS
}

void k_thread_idle() {
	for(;;);
}

void zombie() {
	printf("Rob Zombie\n");
	for(;;);
}

void zombie2() {
	printf("Zombie2\n");
	for(;;);
}

void zombie3() {
	printf("Electric BOOGALOO\n");
	for(;;);
}

thread* k_sched_next() {
	thread* t = current_thread->next;
	current_thread = t;

	if (t == 0)

	return t;
}

/*
Thread1 -> Current Thread (2) -> Thread 3
Thread1 -> CT (2) -> New Thread (4) -> Thread 3 -> Thread 1
*/
void k_thread_add(thread* t) {
	thread* c = current_thread;
	t->next = c->next;
	c->next = t;

}

void k_schedule(regs_t* r) {
	current_thread->esp = r->esp;
	current_thread->eip = r->eip;
	current_thread->ebp = r->ebp;

	thread* t = k_sched_next();

	r->esp = t->esp;
	r->eip = t->eip;
	r->ebp = t->ebp;
	return r;

}

void k_thread_init() {
	k_thread = k_create_thread("kernel idle", k_thread_idle);

	thread* kt2 = k_create_thread("zthread2", zombie2);
	thread* kt3 = k_create_thread("zthre3", zombie3);
	current_thread = k_thread;
	k_thread_add(k_thread);
	k_thread_add(kt2);
	k_thread_add(kt3);
	//k_thread_add(k_thread);

	K_SCHED_ACTIVE = 1;

}