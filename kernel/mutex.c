/*
mutex.c
*/ 

#include <types.h>
#include <mutex.h>

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

void mutex_test() {
	mutex ml = {.lock = 0};

	acquire(&ml);

	printf("acquire: %d\n", ml.lock);

	release(&ml);
	printf("release: %d\n", ml.lock);
}
