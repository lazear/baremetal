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
extern process* get_current_proc();

int fork() {
	acquire(&proclock);

	process* c = get_current_proc();

	process *child = (process*) malloc(sizeof(process));
	/* 
	Now we need to make a new page directory/address space, and 
	copy over the relevant information before we can spawn a child.
	*/

	uint32_t* parent_esp = c->stack;

	uint32_t* pgdir = k_create_pagedir(0x01000000, 256, 0x7);
	k_map_kernel(pgdir);
	

	//page_copy(pgdir, KERNEL_PAGE_DIRECTORY, child, sizeof(process));

	printf("fork() pid %d @ %x\n", nextpid, child);

	memcpy(child, c, sizeof(process));

	
	strcpy(child->name, "fork()");
	child->parent = c;
	child->pagedir = pgdir;
	child->pid = nextpid;
	child->frame->eax = 0;
	sbrk(0x1000);
	ptable[nextpid] = c;
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, &ptable, sizeof(process) * MAX_PROCESS);
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, parent_esp, 0x1000);
	page_copy(pgdir, KERNEL_PAGE_DIRECTORY, child, sizeof(process));

	// page_copy(pgdir, KERNEL_PAGE_DIRECTORY, 0x01000000, 0x7000);


	procinfo(child);
	release(&proclock);
	return nextpid++;
}