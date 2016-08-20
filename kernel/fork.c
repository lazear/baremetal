/*
fork.c
*/

#include <types.h>
#include <proc.h>
#include <mutex.h>

extern mutex proclock;
extern process** ptable;
extern int current_pid;
extern uint32_t* KERNEL_PAGE_DIRECTORY;
extern int nextpid;

int fork() {
	acquire(&proclock);

	process* c = ptable[0];

	int parent =c->pid;

	process *child = (process*) malloc(sizeof(process));
	/* 
	Now we need to make a new page directory/address space, and 
	copy over the relevant information before we can spawn a child.
	*/

	uint32_t* parent_esp = c->stack;
	printf("Parent stack @ 0x%x\n", parent_esp);

	uint32_t* pgdir = k_create_pagedir(0x01000000, 256, 0x7);
	k_map_kernel(pgdir);
	

	//page_copy(pgdir, KERNEL_PAGE_DIRECTORY, child, sizeof(process));

	printf("fork() pid %d\n", 0);

	memcpy(child, c, sizeof(process));

	
	strcpy(child->name, "fork()");
	child->pagedir = pgdir;
	child->pid = nextpid;
	child->frame->eax = 0;

	//printf("Cur pid: %d\tNumber of threads: %d\n", current_pid, nextpid);
	//walk_pagedir(pgdir);
	ptable[nextpid] = c;
	//c->pagedir = pgdir;
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, &ptable, sizeof(process) * MAX_PROCESS);
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, parent_esp, 0x1000);
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, child, sizeof(process));
	//printf("Cur pid: %d\tNumber of threads: %d\n", current_pid, nextpid);
	procinfo(child);
	release(&proclock);
	return nextpid++;
}