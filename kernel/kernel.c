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

int ticks = 0;

char logo[] = {0x5f,0x5f,0x5f,0x2e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5f,0x5f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x2e,0x5f,0x5f,0x20,0x20,0x20,0x0a,0x5c,0x5f,0x20,0x7c,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x2f,0x20,0x20,0x7c,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5f,0x5f,0x20,0x5c,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x5c,0x5f,0x20,0x20,0x5f,0x5f,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x20,0x20,0x5f,0x5f,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5c,0x5f,0x5c,0x20,0x5c,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x20,0x5c,0x2f,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x59,0x20,0x59,0x20,0x20,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x7c,0x20,0x20,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x5f,0x5f,0x0a,0x20,0x7c,0x5f,0x5f,0x5f,0x20,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x7c,0x20,0x20,0x20,0x20,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x5f,0x7c,0x20,0x20,0x2f,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x5f,0x5f,0x2f,0x0a,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20, '\n'};

extern struct tss_entry system_tss;
extern uint32_t K_SCHED_ACTIVE;
extern uint32_t K_SCHED_TIME;

char* timer_buf = 0;
int buffer_set = 0;

void timer(regs_t *r) {

	//cli();
	ticks++;

	if (!(ticks%1)) {
		if (!buffer_set) {
			timer_buf = malloc(8);
			memset(timer_buf, 0, 8);
		}
		buffer_set=1;

		itoa(ticks, timer_buf, 10);

		vga_kputs(timer_buf, 150, 0);

	}

	if (ticks % K_SCHED_TIME == 0)
		//	asm volatile("out %%al, %%dx": :"d"(0x20), "a"(0x20)); // send EoI to master PIC
			return k_schedule(r);
	//sti();
	return r;
}

void wait(int n) {
	int wait_for = ticks + n;
	while (1)
		if (wait_for < ticks)
			break;
	return;
}

uint32_t get_ticks(){
	return ticks;
}

void print_regs(regs_t* r) {
	vga_clear();
	vga_pretty("Register dump\n", VGA_MAGENTA);
	printf("gs: 0x%x\n\n", r->gs);
	printf("fs: 0x%x\n", r->fs);
	printf("es: 0x%x\n", r->es);
	printf("ds: 0x%x\n", r->ds);
	printf("edi: 0x%x\n", r->edi);
	printf("esi: 0x%x\n", r->esi);
	printf("ebp: 0x%x\n", r->ebp);
	printf("esp: 0x%x\n", r->esp);
	printf("ebx: 0x%x\n", r->ebx);
	printf("edx: 0x%x\n", r->edx);
	printf("ecx: 0x%x\n", r->ecx);
	printf("eax: 0x%x\n", r->eax);
	printf("eip: 0x%x\n", r->eip);
	printf("cs: 0x%x\n", r->cs);
	printf("flags: 0x%x\n", r->flags);
	printf("esp3: 0x%x\n", r->esp3);
	printf("ss3: 0x%x\n", r->ss3);

}


// Disable interrupts
void cli() {
	asm volatile("cli");
}

// Enable interrupts
void sti() {
	asm volatile("sti");
}

int keypress = 0;


STREAM* stdin = 0;
extern STREAM* kb;


// keyboard loop thread
int kb_tell = 0;
void event() {
	while(1) {
		if (ftell(kb)) {
			lock();
//			sti();

			char c = fgetc(kb);
			if (c == '\n')
				yield();
			fflush(kb);
			vga_putc(c);
//			fflush(kb);
//			yield();
		}

	}
}

//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled
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
	uint32_t* pagedir = k_mm_init(kernel_end);
	k_paging_init(pagedir);
	k_heap_init();
	// Start timer

	keyboard_install();
	irq_install_handler(0, timer);

	// Start interrupts
	sti();

	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	vga_pretty(logo, VGA_CYAN);

	STREAM* s = k_new_stream(62);
	//stdin = k_new_stream(0x200);

	char d[] = "Michael Lazear (C) 2016";
	int res = k_stream_write(s, d, 10);
	


	k_thread_init(3);
	spawn("/dev/stdin", event);

	//k_heap_test();

	//mm_test();
	mm_debug();
	k_heap_test();

	for(;;);
}

