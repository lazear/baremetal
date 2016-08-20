/*
sched.c
*/

#include <types.h>
#include <x86.h>
#include <proc.h>
#include <mutex.h>



extern uint32_t* KERNEL_PAGE_DIRECTORY;
process** ptable;
int nextpid = 0;
int running = 0;
int current_pid = 0;

mutex proclock = {.lock = 0};

extern void trapret();
extern struct tss_entry system_tss;

void set_tss_esp() {
	system_tss.esp0 = ptable[0]->stack;
}

void kill(int pid) {
	if (!pid) {
		panic("Attempting to kill system process");
		asm volatile("hlt");
		return;
	}
	if (pid >= nextpid) {
		panic("PID does not exist");
		return;
	}

	acquire(&proclock);
	process* p = ptable[pid];
	p->state = 0;
	p->next = ptable[0];

	free(p->stack);
	memset(p, 0, sizeof(process));
	free(p);
	//ptable[pid] = NULL;
	printf("PID %d is kill\n", pid);
	release(&proclock);


}

void die() {
	kill(getpid());
	while(1) {
//		printf("pid %d is a zombie\n", getpid());
		sched();

	}

}

void __exit() {
	die();
}

process* spawn(char* name, void (*fn)() ) {
	acquire(&proclock);
	process* p = (process*) malloc(sizeof(process));
	memset(p, 0, sizeof(process));
	strcpy(p->name, name, 16);

	uint32_t* stack = (uint32_t*) (malloc(0x1000));		// stack will grow down
	uint32_t top = ((uint32_t) stack + 0x1000);			// ebp points to top of stack

	memset(stack, 0, 0x1000);

	stack = top;
	stack -= sizeof(regs_t);

	p->pid = nextpid++;
	p->pagedir = KERNEL_PAGE_DIRECTORY;
	p->state = 1;
	p->frame = stack;

	p->frame->eip = (uint32_t) __exit;
	p->frame->cs = 0x08;
	p->frame->ds = 0x10;
	p->frame->es = 0x10;
	p->frame->fs = 0x10;
	p->frame->gs = 0x10;
	p->frame->flags = 0x202;

	*--stack = (uint32_t) trapret;
	*--stack = (uint32_t) fn;	// EIP
	*--stack = top;				// EBP
	*--stack = 0; 				// EBX
	*--stack = 0; 				// ESI
	*--stack = 0; 				// EDI

	p->stack = stack;
	p->frame->esp = stack;
	ptable[p->pid] = p;

	release(&proclock);
	return p;
}

process* get_current_proc() {
	return ptable[current_pid];
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


/* __wait spins idly - no context changing */
void __wait(int t) {
	int waitfor = get_ticks() + t;
	while(get_ticks() < waitfor) {
		;//yield(); ;
	}
}

/*
wait(int ticks) calls yield once per tick, allows less time
spent spinning through the scheduler */
void wait(int n) {
	while(n--) {
		__wait(1);
		yield();
	}
}



void yield() {
//	ptable[current_pid]->state = 0;
	sched();
}

int getpid() { 
	return current_pid;
}


uint32_t swap(uint32_t* esp) {
	acquire(&proclock);
	static int first = 0;
	if (!first) {
		/*
		If the scheduler is being run for the first time,
		we need to save the current state of the CPU. 
		PID 0 will become the null/system task, and will 
		actually loop back to kernel_initialize */
		ptable[0]->stack = esp;
		first = 1;
		release(&proclock);
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

	release(&proclock);
	k_swap_pd(ptable[current_pid]->pagedir);
	return ptable[current_pid]->stack;


}

void scheduler(uint32_t esp) {
	if (!running++) return esp;
	//printf("%d\n", current_pid);
	return swap(esp);
}

int status(int pid) {
	return ptable[pid]->state;
}
int sched_state() {
	return running;
}

void sysidle() {
	while(1) sched();
}

void procinfo(process* p) {
	printf("%s, pid %d, stack @ %x, state: %d, time %d, link %d\n", p->name, p->pid, p->stack, p->state, p->time, p->next->pid);
}

void list_procs() {
	printf("Current process is %s (%d)\n", ptable[current_pid]->name, current_pid );
	for (int i = 0; i < nextpid; i++)
		procinfo(ptable[i]);
}

void sched_init() {
	ptable = (process*) malloc(sizeof(process) * MAX_PROCESS);

	process* idle = spawn("kernel", sysidle);
	spawn("sched", sysidle);
	auto_link();
	
	running = 1;

	//sched();

}