/*
Michael Lazear, (C) 2007-2016

kernel.c
*/

#include <kernel.h>
#include <vga.h>
#include <x86.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <mutex.h>
#include <proc.h>

char logo[] = {0x5f,0x5f,0x5f,0x2e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5f,0x5f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x2e,0x5f,0x5f,0x20,0x20,0x20,0x0a,0x5c,0x5f,0x20,0x7c,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x2f,0x20,0x20,0x7c,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5f,0x5f,0x20,0x5c,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x5c,0x5f,0x20,0x20,0x5f,0x5f,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x20,0x20,0x5f,0x5f,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5c,0x5f,0x5c,0x20,0x5c,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x20,0x5c,0x2f,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x59,0x20,0x59,0x20,0x20,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x7c,0x20,0x20,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x5f,0x5f,0x0a,0x20,0x7c,0x5f,0x5f,0x5f,0x20,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x7c,0x20,0x20,0x20,0x20,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x5f,0x7c,0x20,0x20,0x2f,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x5f,0x5f,0x2f,0x0a,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20, '\n'};

static mutex key_mutex = { .lock = 0 };

extern STREAM* kb;

void event_loop() {
	while(1) {

		acquire(&key_mutex);
		if (ftell(kb)) {
			//fflush(kb);
			//list_procs();
			vga_kputc(fgetc(kb), 150, 1);
			fflush(kb);


		}
		release(&key_mutex);
		yield();
	}
}



extern void switch_to_user(void* (fn)(), uint32_t esp);
//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled

uint32_t KERNEL_END = 0;

extern process** ptable;

void kernel_initialize(uint32_t kernel_end) {

	/*
	1.	Initialize physical memory manager, with private heap @ kernel_end to 2MB
			- Bitmap has the first 2MB (512 bits) marked as used, will not be allocated
			- Allows k_page_alloc()
	2.	Initialize paging, passing the reserved first page directory address
	3.	Initialize heap management with malloc, free, and sbrk.
			- Utilizes both k_page_alloc() and k_paging_map() to generate a continous
				virtual address space.
			- Public heap starts at 3GB. This should be changed when higher half is implemented
	4.	Todo - initialize multithreading.
	*/
	KERNEL_END = kernel_end;
	uint32_t* pagedir = k_mm_init(kernel_end);
	k_paging_init(pagedir);
	k_heap_init();
	// Start timer
	//tss_flush();
	//pic_init();
	keyboard_install();
	timer_init();
	syscall_init();
	// Initial start interrupts.
	sti();

	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	vga_pretty(logo, VGA_CYAN);

	sched_init();
	printf("Back in kernel-init\n");

	extern uint32_t KERNEL_PAGE_DIRECTORY;

//	spawn("loop", event_loop);

	//fork();
	while(1) yield();

	for(;;);
}

