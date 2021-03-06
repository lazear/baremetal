/*
x86.c
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
#include <traps.h>
#include <smp.h>

/****************************************
*		        ASM Template			*
* asm ("operands, %%reg, val"	    	*
*			:outputs					*
*			:inputs)					*
* outputs example: "=r"(val)			*
* inputs example: "r"(10)				*
*****************************************/

int set_eflags(int val) {
	int flags;
	asm volatile("			\
				 pushf;		\
				 pop %%eax;	\
				 mov %%eax, %0;	" : "=r"(flags));
	flags |= val;
	asm volatile("				\
				 mov %0, %%eax;	\
				 push %%eax;	\
				 popf;" : :"r"(flags));
	return flags;
}

int get_eflags(void) {
	int flags;
	asm volatile("			\
				 pushf;		\
				 pop %%eax;	\
				 mov %%eax, %0;	" : "=r"(flags));
	return flags;
}


uint8_t inb(uint16_t port) {
  // "=a" (result) means: put AL register in variable result when finished
  // "d" (_port) means: load EDX with _port
	unsigned char result;
	asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

void outb(uint16_t port, uint16_t data) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

uint16_t inw(uint16_t port) {
	unsigned short result;
	asm volatile ("inw %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

void outw(uint16_t port, uint16_t data) {
	asm volatile("outw %1, %0" : :"dN"(port), "a"(data));
}

void outl(uint16_t port, uint32_t data) {
	asm volatile("outl %1, %0" : :"dN"(port), "a"(data));
}

uint64_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0" : "=a"(ret) : "dN"(port));
}

/*
cld						clear direction flag
mov ecx, cnt 			set loop counter
rep outsl port, addr	repeats outsl ecx times
*/
void outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}




static inline void lidt(struct gatedesc *p, int size) {
	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	asm volatile("lidt (%0)" : : "r" (pd));
}

extern void gdt_flush();

static inline void lgdt(struct segdesc *p, int size) {

	// 16:32 bit pointer is required for lgdt/lidt
	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;
	asm volatile("lgdt (%0)" : : "r" (pd));
	gdt_flush();
}

static inline void ltr(uint16_t sel) {
	asm volatile("ltr %0" : : "r" (sel));
}

void idt_init() {
	pic_init();
	for (int i = 0; i < 256; i ++)
		SETGATE(idt[i], 0, 0x8, vectors[i], 0);
	// Make syscall accessible from userland
	SETGATE(idt[T_SYSCALL], 1, 0x8, vectors[T_SYSCALL], DPL_USER);
	lidt(idt, sizeof(idt));
}


extern uint32_t stack_top[];
extern uint32_t stack_bottom[];

void gdt_init_cpu(char id) {

	struct cpu* c;
	c = &cpus[id];

	c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);			// CS = 0x8
	c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);					// SS = 0x10
	c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);		// CS = 0x20
	c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);			// SS = 0x28
	c->gdt[SEG_KCPU]  = SEG(STA_W, &c->cpu, sizeof(struct cpu), 0);

	c->gdt[SEG_TSS] = SEG(STS_T32A, &system_tss, sizeof(system_tss)-1, 0);
	c->gdt[SEG_TSS].s = 0;


	lgdt(c->gdt, sizeof(c->gdt));

	asm volatile("mov %0, %%gs" : : "r"(SEG_KCPU<<3));

	//system_tss.esp0 = cpus[id].stack;
	ltr(SEG_TSS<<3);

	

}


// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void gdt_init(void)
{

	cpus[0].id = 0;
	cpus[0].stack = &stack_top;
	cpus[0].cpu = &cpus[0];
	cpus[0].ncli = 0;
	cpu = &cpu[0];


	gdt_init_cpu(0);


	system_tss.cs = SEG_KCODE << 3;
	system_tss.ss0 = SEG_KDATA << 3;

	int esp = 0;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	system_tss.esp0 = esp;
	//ltr(SEG_TSS<<3);

}


// Disable interrupts
void cli() {
    asm volatile("cli");
}

// Enable interrupts
void sti() {
    asm volatile("sti");
}


void print_regs(regs_t* r) {
    //vga_clear();
   // r = (uint32_t)r + 4;
    printf("Register dump\n");
    printf("gs: 0x%x\n", r->gs);
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