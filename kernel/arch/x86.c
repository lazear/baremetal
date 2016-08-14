/*
vga.c
Michael Lazear, 2007-2016

Implementation of x86 interrupts and inline assembly for baremetal
Adapted/taken from Bran's Kernel Dev tutorial.
*/



#include <types.h>
#include <x86.h>

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


struct idt_entry idt[256];
struct idt_ptr idt_pointer;

struct gdt_entry gdt[8];
struct gdt_ptr gdt_pointer;


/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
    irq_routines[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}

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

void irq_handler(struct regs *r)
{

	if (r->int_no < 32) {
		printf("Exception: %d", r->int_no);
		return;
	}

    /* This is a blank function pointer */
    void (*handler)(struct regs *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];

    if (handler)
    {
		//printf("IRQ Calling handler: 0x%x\n", handler);
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outportb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outportb(0x20, 0x20);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	/* The interrupt routine's base address */
	idt[num].base_lo = (base & 0xFFFF);
	idt[num].base_hi = (base >> 16) & 0xFFFF;

	/* The segment or 'selector' that this IDT entry will use
	*  is set here, along with any access flags */
	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = flags;
}

void idt_init() {
	/* Sets the special IDT pointer up, just like in 'gdt.c' */
	idt_pointer.limit = (sizeof (struct idt_entry) * 256) - 1;
	idt_pointer.base = &idt;

	/* Clear out the entire IDT, initializing it to zeros */
//	memset(&idt, 0, sizeof(struct idt_entry) * 256);

/*
	for (int i = 0; i < sizeof(struct idt_entry) * 256; i++) {
		int* pointer = &idt;
		pointer[i] = 0;
	}
*/
	/* Add any new ISRs to the IDT here using idt_set_gate */
    idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);

    idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);

    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);

    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);

    // call an IRQ re-map

    irq_remap();

    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);

    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
	/* Points the processor's internal register to the new IDT */
	idt_load();
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


