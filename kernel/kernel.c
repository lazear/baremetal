/*
kernel.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
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
#include <assert.h>
#include <ide.h>
#include <ext2.h>

char logo[] = {0x5f,0x5f,0x5f,0x2e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5f,0x5f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x2e,0x5f,0x5f,0x20,0x20,0x20,0x0a,0x5c,0x5f,0x20,0x7c,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x20,0x5f,0x5f,0x5f,0x5f,0x5f,0x2f,0x20,0x20,0x7c,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5f,0x5f,0x20,0x5c,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x5c,0x5f,0x20,0x20,0x5f,0x5f,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x5f,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x20,0x20,0x20,0x5f,0x5f,0x5c,0x5f,0x5f,0x20,0x20,0x5c,0x20,0x7c,0x20,0x20,0x7c,0x20,0x20,0x0a,0x20,0x7c,0x20,0x5c,0x5f,0x5c,0x20,0x5c,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x20,0x5c,0x2f,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x59,0x20,0x59,0x20,0x20,0x5c,0x20,0x20,0x5f,0x5f,0x5f,0x2f,0x7c,0x20,0x20,0x7c,0x20,0x20,0x2f,0x20,0x5f,0x5f,0x20,0x5c,0x7c,0x20,0x20,0x7c,0x5f,0x5f,0x0a,0x20,0x7c,0x5f,0x5f,0x5f,0x20,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x7c,0x20,0x20,0x20,0x20,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x5f,0x7c,0x20,0x20,0x2f,0x5c,0x5f,0x5f,0x5f,0x20,0x20,0x3e,0x5f,0x5f,0x7c,0x20,0x28,0x5f,0x5f,0x5f,0x5f,0x20,0x20,0x2f,0x5f,0x5f,0x5f,0x5f,0x2f,0x0a,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5c,0x2f,0x20,0x20,0x20,0x20,0x20, '\n'};

static mutex key_mutex = { .lock = 0 };

extern STREAM* kb;

extern void scheduler();
void scheduler() {}
extern void switch_to_user(void* (fn)(), uint32_t esp);
//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts disabled

extern uint32_t* KERNEL_PAGE_DIRECTORY;
uint32_t KERNEL_END = 0;
extern uint32_t _init_pt[];
extern uint32_t _init_pd[];
extern uint32_t stack_top;
extern uint32_t stack_bottom;


#define DEBUG 0

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
	k_mm_init(kernel_end);
	k_heap_init();
	k_paging_init(_init_pd);

	keyboard_install();
	timer_init();
	syscall_init();

	sti();

	vga_setcolor(VGA_COLOR(VGA_WHITE, VGA_BLACK));
	vga_clear();
	vga_pretty(logo, VGA_LIGHTGREEN);
/*
	printf("Memory map:\n");
	printf("Kernel end:     0x%x\n", kernel_end);
	printf("Stack top:      0x%x\n", &stack_top);
	printf("Stack bottom:   0x%x\n", &stack_bottom);
	printf("Page directory: 0x%x\nPage table:     0x%x\n", _init_pd, _init_pt);
	printf("Heap brk:       0x%x\n", heap_brk());

*/
	_paging_map(&_init_pd, k_page_alloc(), 0xD0000000, 0x3);
	// Page fault test
	char* ptr = 0xD0000000;
	*ptr = 'A';
	ide_init();
	buffer_init();

	//lsroot();
	buffer* b = buffer_read(1, 8192+EXT2_SUPER+1);
	block_group_descriptor* sb = (block_group_descriptor*) b->data;
	
//	assert(sb->magic == EXT2_MAGIC);
	//bg_dump(sb);
	lsroot();
//	bg_dump(ext2_blockdesc(1) + 14*sizeof(block_group_descriptor));
	//printf("time %x\n", time());
	//ctime(time());
	//ext2_open_dir(28);

/*	ext2_inode(1, 28001);

	char* data = ext2_open(ext2_lookup("home"));
	dirent* d = (dirent*) data;
	
	do{
		d->name[d->name_len] = '\0';
		printf("name: %s\ttype %d\n", d->name, d->file_type);

		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);*/


	for(;;);
}

