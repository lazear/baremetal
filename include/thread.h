// thread.h

#include <types.h>
#ifndef __baremetal_thread__
#define __baremetal_thread__


typedef struct THREAD {
	char* name;
	uint32_t pid;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eip;
	uint32_t cr3;
	int state;
	uint32_t time;
	struct THREAD* next;

} thread;

void k_add_thread(thread* t);
thread* k_find_thread(int pid);
void k_schedule(regs_t* r) ;
void fork();

#endif