
#include <types.h>

void syscall_handler(void)
{
	uint32_t eax, ebx, ecx, edx;
	asm volatile("mov %%eax, %0" : "=r"(eax));
	asm volatile("mov %%ebx, %0" : "=r"(ebx));
	asm volatile("mov %%ecx, %0" : "=r"(ecx));
	asm volatile("mov %%edx, %0" : "=r"(edx));
	switch(eax)
	{

	default:
		printf("syscall 0x80, eax 0x%x\n", eax);
		break;
	}
		
	asm volatile("sti");
}

void syscall_init(void)
{
	idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0x8e);
}