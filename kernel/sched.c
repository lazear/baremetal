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

typedef struct process_t {
	char name[16];
	int state;
	int pid;
	int quantum;
	int spins;
	uint32_t* stack;
	uint32_t* pagedir;
	struct process_t* next;
} process;

int nextpid = 0;
process** ptable;

void backup() {
	printf("WOAHHHH! PID %d has left the building\n", getpid());
	sched();
	printf("Whats happening?\n");
	for(;;);
}

process* spawn(char* name, void (*fn)() ) {

	process* p = (process*) malloc(sizeof(process));
	memset(p, 0, sizeof(process));
	strcpy(p->name, name, 16);
	p->pid = nextpid++;
	p->pagedir = NULL;
	p->next = NULL;
	p->state = 1;
	uint32_t* stack = (uint32_t*) (malloc(0x1000));		// stack will grow down
	uint32_t top = ((uint32_t) stack + 0x1000);			// ebp points to top of stack

	memset(stack, 0, 0x1000);

	stack = top;
	stack -= sizeof(regs_t);
	*--stack = (uint32_t) backup;
	*--stack = (uint32_t) fn;	// EIP
	*--stack = top;				// EBP
	*--stack = 0; 				// EBX
	*--stack = 0; 				// ESI
	*--stack = 0; 				// EDI

	p->stack = stack;
	ptable[p->pid] = p;

	return p;
}


int iq = 0;

void fn1() {
	printf("FN1\n");
	sched();
}

void fn2() {
	printf("FN2\n");
	printf("Hello FN2 - PID %d\n", getpid());
	//printf("WE MADE IT");
	int waitfor = get_ticks() + 15;
	while(get_ticks() < waitfor) {
		;
	}
	fn1();
	printf("Back to %d\n",getpid());
	yield();
//	for(;;);
//	yield();
//	printf("After yeild!\n");
}


int running = 0;
int current_pid = 0;
int first = 0;


process* idle = 0;
process* a = 0;
process* b = 0;

void yield() {
	ptable[current_pid]->state = 0;
	procinfo(ptable[current_pid]);
	sched();
}

int getpid() { return current_pid; }


uint32_t swap(uint32_t* esp) {

	if (!first) {
		ptable[0]->stack = esp;
		first = 1;
		printf("Saving CPU state in ptable[0], %x\n", esp);
		return ptable[++current_pid]->stack;

	} else
		ptable[current_pid]->stack = esp;
	

	if (ptable[current_pid]->state)
		if (current_pid == nextpid - 1) {
		//	printf("Round robin\n");
			current_pid = 0;
		}
		else {
			current_pid++;
		}
	else {
		printf("PID %d state 0\n", current_pid);
		current_pid++;

		return swap(esp);
	}

	printf("Swap to %d\n", current_pid);

	return ptable[current_pid]->stack;
}

void scheduler(uint32_t esp) {
	//cli();

	if (!running) return esp;
//	printf("%x\n", esp);
	cli();
	set_eflags(0x202);
	uint32_t ret = swap(esp);
	sti();
	return ret;
	//return esp;
}

extern void sched_handler();

void sysidle() { for(;;); }

void procinfo(process* p) {
	printf("%s, pid %d, stack @ %x, state: %d\n", p->name, p->pid, p->stack, p->state);
}

void sched_init() {
	ptable = (process*) malloc(sizeof(process) * 16);

	idle = spawn("Idle", sysidle);
	a = spawn("A", fn2);
	spawn("A2", fn2);
	spawn("A3", fn2);
	
	running = 1;
	printf("Cur pid %d, Nextpid %d\n", current_pid, nextpid);
	sched();


	printf("Hullo?");

	for (int i = 0; i < nextpid; i++)
		procinfo(ptable[i]);
}