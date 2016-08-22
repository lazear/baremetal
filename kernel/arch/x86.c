/*
vga.c
Michael Lazear, 2007-2016

Implementation of x86 interrupts and inline assembly for baremetal
Adapted/taken from Bran's Kernel Dev tutorial and the MIT xv6 project.
*/



#include <types.h>
#include <x86.h>
#include <traps.h>

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


uint8_t inportb(uint16_t port) {
  // "=a" (result) means: put AL register in variable result when finished
  // "d" (_port) means: load EDX with _port
  unsigned char result;
   asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
  return result;
}

void outportb(uint16_t port, uint16_t data) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

uint16_t inportw(uint16_t port) {
	unsigned short result;
	asm volatile ("inw %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

void outportw(uint16_t port, uint16_t data) {
	asm volatile("outw %1, %0" : :"dN"(port), "a"(data));
}

void outportl(uint16_t port, uint32_t data) {
	asm volatile("outl %1, %0" : :"dN"(port), "a"(data));
}

uint64_t inportl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0" : "=a"(ret) : "dN"(port));
}



struct gdt_entry gdt[8];
struct gdt_ptr gdt_pointer;


void irq_remap(void)
{
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}


struct gatedesc idt[256];

static inline void lidt(struct gatedesc *p, int size) {
	volatile uint16_t pd[3];

	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	asm volatile("lidt (%0)" : : "r" (pd));
}


void idt_init() {
	pic_init();
	//irq_remap();
	for (int i = 0; i < 256; i ++)
		SETGATE(idt[i], 0, 0x8, vectors[i], 0);

	SETGATE(idt[T_SYSCALL], 1, 0x8, vectors[T_SYSCALL], DPL_USER);
	

	/* Points the processor's internal register to the new IDT */

	lidt(idt, sizeof(idt));
}

extern void trap();
/* Should switch back to using an array of handlers */
void trap(regs_t* r) {
	switch (r->int_no) {
		case IRQ0 + IRQ_TIMER:
			timer(r);
		case IRQ0 + IRQ_KBD:
			keyboard_handler(r);
	}

	if (r->int_no > 40)
		outportb(0xA0, 0x20);
	outportb(0x20, 0x20);
}

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
   	gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

// Statically allocate, since we just need one entry
struct tss_entry system_tss;
extern uint32_t* read_stack_pointer();

void tss_init() {
    uint32_t base = (uint32_t) &system_tss;         // address of tss
    uint32_t limit = base + sizeof(system_tss);     // limit of selector

    gdt_set_gate(5, base, limit, 0xE9, 0x00);     // setup gdt entry
    memset(&system_tss, 0, sizeof(system_tss));     // clear the tss

    system_tss.ss0  = 0x10;
    system_tss.esp0 = 0x0;
    system_tss.cs   = 0xb;
    system_tss.ss   = system_tss.ds = system_tss.es = system_tss.fs = system_tss.gs = 0x13;     // RPL = 3
}

void gdt_init()
{
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gdt_pointer.base = &gdt;

    gdt_set_gate(0, 0, 0, 0, 0);					// 0x00
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);		// 0x08 Code, PL0
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);		// 0x10 Data, PL0
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);		// 0x18 Code, PL3
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);		// 0x20 Data, PL3

	tss_init();
    gdt_flush();

    tss_flush();
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