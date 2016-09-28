/*
syscall.c
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
#include <types.h>
#include <x86.h>

extern void syscall_handler();

extern uint32_t* KERNEL_PAGE_DIRECTORY();


void syscall(regs_t *r)
{
/*	uint32_t eax, ebx, ecx, edx;
	asm volatile("mov %%eax, %0" : "=r"(eax));
	asm volatile("mov %%ebx, %0" : "=r"(ebx));
	asm volatile("mov %%ecx, %0" : "=r"(ecx));
	asm volatile("mov %%edx, %0" : "=r"(edx));*/
	int ret;
	char* s;
	switch(r->eax)
	{
	case 0:
		printf("exit()\n");
		printf("kernel pd %x\n", KERNEL_PAGE_DIRECTORY);

		pushcli();
		k_swap_pd(KERNEL_PAGE_DIRECTORY);
		for(;;);
		break;
	case 1:	// syscall write()
		printf("write(%s)\n", (char*) r->edx);
		break;
	case 2:
		vga_putc(r->edx);
		break;
	case 8:
		sys_sbrk(&cp_mmap, r->ebx);
		r->eax = cp_mmap.brk;
		break;
	default:
		printf("syscall 0x80, eax 0x%x\n", r->eax);
		//ret = fork();
		break;
	}
	//print_regs(cp);
	//r->eax = 0xDEAD;
	//r->eax = 0xDEADBEEF;

}

void syscall_init(void)
{
	//idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0x8e);
}