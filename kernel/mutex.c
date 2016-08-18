/*
mutex.c
*/ 

#include <types.h>
#include <mutex.h>


// Schedule-spinlock mutex
void acquire(mutex *m) {
	while(m->lock)
		;
	m->lock = 1;
//	printf("PID %d acquired %x\n", getpid(), m);
}

void release(mutex *m) {
	if (!m->lock)
		panic("mutex not locked");
	m->lock = 0;
//	printf("PID %d released %x\n", getpid(), m);
}



static int ncli = 0;

void pushcli() {

	asm volatile("cli");
	ncli++;
}

void popcli() {

	if(--ncli < 0)
		panic("popcli without push");
	if(ncli == 0)
		asm volatile("sti");
}