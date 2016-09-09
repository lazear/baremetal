
#include <types.h>
#include <x86.h>
#include <proc.h>

extern void syscall_handler();



void syscall(regs_t *r)
{
/*	uint32_t eax, ebx, ecx, edx;
	asm volatile("mov %%eax, %0" : "=r"(eax));
	asm volatile("mov %%ebx, %0" : "=r"(ebx));
	asm volatile("mov %%ecx, %0" : "=r"(ecx));
	asm volatile("mov %%edx, %0" : "=r"(edx));*/
	process* c = get_current_proc();
	c->frame = r;
	int ret = 10;
	switch(r->eax)
	{

	default:
		printf("syscall 0x80, eax 0x%x\n", r->eax);
		ret = fork();
		break;
	}
	//print_regs(cp);
	r->eax = ret;
	//r->eax = 0xDEADBEEF;
//	asm volatile("hlt");
}

void syscall_init(void)
{
	idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0x8e);
}