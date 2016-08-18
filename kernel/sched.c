/*
sched.c
*/

#include <types.h>
#include <x86.h>
#include <threads.h>

#define MAX_PROCESS		256

process** ptable;
int nextpid = 0;
int running = 0;
int current_pid = 0;


void kill(int pid) {
	if (!pid) {
		panic("Attempting to kill system process");
		return;
	}
	if (pid > nextpid) {
		panic("PID does not exist");
	}

	pushcli();
	process* p = ptable[pid];
	p->state = 0;
	p->next = ptable[0];

	free(p->stack);
	memset(p, 0, sizeof(process));
	free(p);
	ptable[pid] = NULL;
	printf("PID %d is kill\n", pid);
	popcli();

}

void die() {
	kill(getpid());

}

void backup() {
	int pid = getpid();

	kill(pid);
	while(1) {
//		printf("pid %d is a zombie\n", getpid());
		sched();

	}
}

process* spawn(char* name, void (*fn)() ) {
	pushcli();
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

	if(!p->pid) {
		ptable[p->pid - 1]->next = p;
		p->next = ptable[0];
	} else {
		// PID 0, link to self
		p->next = p;
	}


	popcli();
	return p;
}

/* Insert process b after process a */
void insert_link(process* a, process* b) {
	if (!b->next)
		b->next = a;
	if (!a->next)
		a->next = b;
	else {
		// A has a next link
		b->next = a->next;
		a->next = b;
	}
}

/* automatically links processes in order of PID */
void auto_link() {
	for (int i = 0; i < nextpid -1 ; i++) {
		ptable[i]->next = ptable[i+1];
	}
	ptable[nextpid-1]->next = ptable[0];	// last entry links back to first
}

void prioritize(int pid) {
	pushcli();
	insert_link(ptable[current_pid], ptable[pid]);
	popcli();
}

int iq = 0;

void wait(int t) {
	int waitfor = get_ticks() + 35;
	while(get_ticks() < waitfor) {
		yield();
	}
}

int i = 0;
void fn2() {
	printf("Hello FN2 - PID %d\n", getpid());
	wait(20);
}


void fn1() {
	while(1)
		yield();	
}


void yield() {
//	ptable[current_pid]->state = 0;
	sched();
}

int getpid() { return current_pid; }


uint32_t swap(uint32_t* esp) {
	pushcli();
	static int first = 0;
	if (!first) {
		/*
		If the scheduler is being run for the first time,
		we need to save the current state of the CPU. 
		PID 0 will become the null/system task, and will 
		actually loop back to kernel_initialize */
		ptable[0]->stack = esp;
		first = 1;
		popcli();
		return ptable[++current_pid]->stack;

	} else {
		/* If the scheduler's already been running, save the context
		of the current task */
		ptable[current_pid]->stack = esp;
	}
	

	current_pid++;
	if (current_pid == nextpid)		// round robin style
		current_pid = 0;
	while (!ptable[current_pid]->state) {
		current_pid++;
		if (current_pid == nextpid)		// round robin style
			current_pid = 0;

	}
	ptable[current_pid]->time++;
	popcli();
	return ptable[current_pid]->stack;


}

void scheduler(uint32_t esp) {
	if (!running) return esp;
//	vga_putc('S');
	return swap(esp);
}

extern void sched_handler();

void sysidle() { for(;;); }

void procinfo(process* p) {
	printf("%s, pid %d, stack @ %x, state: %d, time %d, link %d\n", p->name, p->pid, p->stack, p->state, p->time, p->next->pid);
}

void list_procs() {
	
	for (int i = 0; i < nextpid; i++)
		procinfo(ptable[i]);
}

void sched_init() {
	ptable = (process*) malloc(sizeof(process) * MAX_PROCESS);

	process* idle = spawn("sys-d", sysidle);
	process* a = spawn("A1", fn2);
	spawn("A2", fn2);
	spawn("B1", fn1);
	auto_link();
	list_procs();
	
	running = 1;
	printf("Cur pid: %d\tNumber of threads: %d\n", current_pid, nextpid);
	//sched();


	printf("Hullo?");

}