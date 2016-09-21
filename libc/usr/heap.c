#include <types.h>

uint32_t HEAP_BASE = 0;
uint32_t HEAP_LOW	=	0;


int syscall_one(int eax, int ebx) {
	int ret = 0;
	asm volatile("mov %0, %%ebx; mov %1, %%eax" : : "b"(ebx), "a"(eax));
	asm volatile("int $0x80");
	asm volatile("mov %%eax, %0" : "=a"(ret));
	return ret;
}

void puts(char* s) {
	syscall_one(1, s);
}


void heap_init() {
	HEAP_LOW = syscall_one(8, 0);
	HEAP_BASE = HEAP_LOW;
}

void* malloc(size_t n) {
	HEAP_LOW = (uint32_t) HEAP_LOW + n;
	if (HEAP_LOW >= (HEAP_BASE + 0x1000)) {
		HEAP_BASE = syscall_one(8, (HEAP_LOW - HEAP_BASE));
	}
	
	return (void*) (HEAP_LOW - n);
}

void free(void* ptr) {
	return;
}


void crt_initialize() {
	heap_init();
}