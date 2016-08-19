// thread.h

#include <types.h>
#ifndef __baremetal_thread__
#define __baremetal_thread__


typedef struct process_t {
	char name[16];
	int state;						
	int pid;
	int quantum;
	int time;
	uint32_t sz;
	uint32_t* stack;
	uint32_t* pagedir;
	struct process_t* next;
	struct process_t* parent;
} process;

typedef struct THREAD {
	char* name;
	uint32_t pid;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eip;
	uint32_t cr3;
	int state;
	uint32_t time;
	struct THREAD* prev;
	struct THREAD* next;
	regs_t *regs;

} thread;


#endif