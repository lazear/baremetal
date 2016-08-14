/*
task.c
*/

#include <types.h>
#include <x86.h>
#include <thread.h>

uint32_t K_SCHED_TIME = 0;
uint32_t K_SCHED_ACTIVE = 0;
uint32_t K_THREAD_COUNT = 0;

int K_CURRENT_PID = -1;

thread* current_thread = 0;

thread* k_create_thread(char* name, void (*fn)() ) {
	cli();
	uint32_t* stack = malloc(0x1000) + 0x1000; // point to top of stack

	thread* t = (thread*) malloc(sizeof(thread));
	memset(t, 0, sizeof(thread));

	t->name = name;
	t->pid = K_THREAD_COUNT++;
	t->eip = (uint32_t) fn;
	t->esp = (uint32_t) stack - 0x1000;		// point to lowest address
	t->ebp = (uint32_t) stack;				// point to highest address of stack
	t->state = 0;							// Task is created but not running
	t->next = NULL;
	t->time = 0;

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

	//printf("Thread %s created, with PID= %d\n", t->name, t->pid);
	sti();
	return t;
}


thread* k_sched_next() {
	if (current_thread->next->state){
		thread* t = current_thread->next;
			//	printf("Scheduling pid %d\n", t->pid);
		current_thread = t;
		return t;
	}
	return current_thread;
}

/*
Thread1 -> Current Thread (2) -> Thread 3
Thread1 -> CT (2) -> New Thread (4) -> Thread 3 -> Thread 1
*/
void k_add_thread(thread* t) {
	t->state = 1;
	//printf("Attempting to add thread %d: %s\n", t->pid, t->name);
	if (current_thread == 0) {
	//	printf("Now the current thread\n");
		t->next = t;
		t->prev = t;
		current_thread = t;
		K_CURRENT_PID = t->pid;
		return;
	}
	
	thread* tmp = current_thread;
	thread* tmp_next = current_thread->next;
	if (tmp_next){
		// there is already a link found, we will insert
	//	printf("Link found %d -> %d\n", tmp->pid, tmp_next->pid);
		tmp->next = t;
		t->prev = tmp;
		t->next = tmp_next;
		tmp_next->prev = t;
	//	printf("New link: %d -> %d -> %d\n", tmp->pid, tmp->next->pid, tmp->next->next->pid);

	} else {
		tmp->next = t;
		t->prev = tmp;
	//	printf("Pid %d is now next after %d\n", tmp->next->pid, tmp->pid);
		t->next = k_find_thread(0);
	}

}

thread* k_find_thread(int pid) {
	thread *tmp = current_thread;
	while (tmp->pid != pid)
		tmp = tmp->next;
	return tmp;
}


void k_schedule(regs_t* r) {

	// This is where the error is... first thread won't run basically.
	// Because the old data is overwriting it before it's scheduled.
	// Save stack values
	if (K_CURRENT_PID < 0){
		printf("ERR - NO SCHED, CURRENT PID %x\n", K_CURRENT_PID);

		return r;
	}
	if (!K_SCHED_ACTIVE)
		return r;

	current_thread->esp = r->esp;
	current_thread->eip = r->eip;
	current_thread->ebp = r->ebp;
	current_thread->time++;
	// Switch to a new thread
	thread* t = k_sched_next();
	if(t->state == 0)
		k_schedule(r);

	vga_kputs(t->name, 150-strlen(t->name), 1);

	K_CURRENT_PID = t->pid;
	// load new stack values
	r->esp = t->esp;
	r->eip = t->eip;
	r->ebp = t->ebp;

	return r;

}

int i = 0;

void z() {
	while(1) {

	}
}

void z2() {
	while(1) {
		if (get_ticks() > 200) {
			die();
		}
	}

}
void k_list_threads() {
	thread* t = current_thread;
	for (int i = 0; i < K_THREAD_COUNT; i++) {
		printf("PID: %d Name: %s Time: %d State %d\n", t->pid, t->name, t->time, t->state);
		if (t->next)
			t = t->next;
	}
}

void null_thread() {
	for(;;);
}

void k_gpf_handler(regs_t* r) {
	print_regs(r);
	vga_pretty("General Protection Fault!!!", 0x2);
	asm volatile("hlt");
}

void die() {
	cli();
	k_sched_state(0);

	thread* t = current_thread;
	printf("die %s %d\n", t->name, t->pid);

	t->prev->next = t->next;

	t->state = 0;
//	K_THREAD_COUNT--;


	free(t->ebp);	
	free(t);

	k_sched_state(1);
	sti();

	for(;;);
}

void yield() {
	k_sched_state(1);
//	sti();
}

void lock() {
//	cli();
	k_sched_state(0);
	//sti();
}

thread* k_fork(int pid) {
	thread* parent = k_find_thread(pid);
	thread* null = k_create_thread("null", NULL);
	int new_pid = null->pid;
	memcpy(null, parent, sizeof(thread));
	null->pid = new_pid;

//	printf("Forked: %s to %s\n", parent->name, null->name);
	k_add_thread(null);
	return null;

}

void fork() {
	k_fork(current_thread->pid);
}

int spawn(char* name, void (*fn)()) {
	k_add_thread(k_create_thread(name, fn));
}

extern int keypress;

void event_loop() {
	printf("Event loo\n");
	die();
	printf("Can you see this?");
}

void k_sched_state(int state) {
	K_SCHED_ACTIVE = state;
}



void k_thread_init(int timing) {

	cli();

	idt_set_gate(6, k_gpf_handler , 0x08, 0x8E);	// invalid opcode
	idt_set_gate(13, k_gpf_handler , 0x08, 0x8E);	// gpf


	spawn("Idle", null_thread);
	//spawn("Main", event_loop);
	//spawn("Zom2", z2);
	//spawn("Zom1", z);
	k_list_threads();
	//current_thread = k_thread;
	//K_CURRENT_PID = 1;
	K_SCHED_ACTIVE = 1;
	K_SCHED_TIME = timing;
	sti();

}