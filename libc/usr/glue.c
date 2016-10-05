#include <types.h>

#ifndef _LIBK_

uint32_t HEAP_BASE = 0;
uint32_t HEAP_LOW	=	0;

int syscall_one(int eax, int ebx) {
	int ret = 0;
	asm volatile("mov %0, %%ebx; mov %1, %%eax" : : "b"(ebx), "a"(eax));
	asm volatile("int $0x80");
	asm volatile("mov %%eax, %0" : "=a"(ret));
	return ret;
}

int syscall_two(int eax, int ebx, int ecx) {
	int ret = 0;
	asm volatile("mov %0, %%ecx" : : "c"(ecx));
	asm volatile("mov %0, %%ebx; mov %1, %%eax" : : "b"(ebx), "a"(eax));
	asm volatile("int $0x80");
	asm volatile("mov %%eax, %0" : "=a"(ret));
	return ret;
}

int syscall_three(int eax, int ebx, int ecx, int edx) {
	int ret = 0;
	asm volatile("mov %0, %%ecx" : : "c"(ecx));
	asm volatile("mov %0, %%edx" : : "c"(edx));
	asm volatile("mov %0, %%ebx; mov %1, %%eax;" : : "b"(ebx), "a"(eax));
	asm volatile("int $0x80");
	asm volatile("mov %%eax, %0" : "=a"(ret));
	return ret;
}

char* open(char* filename) {
	return (char*) syscall_one(0x10, filename);
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
#endif

void crt_initialize() {
	heap_init();
}

