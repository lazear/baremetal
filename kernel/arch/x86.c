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


struct segdesc gdt[8];
struct gatedesc idt[256];
struct tss_entry system_tss;

static inline void lidt(struct gatedesc *p, int size) {
	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	asm volatile("lidt (%0)" : : "r" (pd));
}


static inline void lgdt(struct segdesc *p, int size) {

	volatile uint16_t pd[3];
	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;
	asm volatile("lgdt (%0)" : : "r" (pd));
}

static inline void ltr(uint16_t sel) {
	asm volatile("ltr %0" : : "r" (sel));
}

void idt_init() {
	pic_init();
	//irq_remap();
	for (int i = 0; i < 256; i ++)
		SETGATE(idt[i], 0, 0x8, vectors[i], 0);
	// Make syscall accessible from userland
	SETGATE(idt[T_SYSCALL], 1, 0x8, vectors[T_SYSCALL], DPL_USER);
	lidt(idt, sizeof(idt));
}


void tss_init() {
    uint32_t base = (uint32_t) &system_tss;         // address of tss
    uint32_t limit = base + sizeof(system_tss);     // limit of selector

   // gdt_set_gate(5, base, limit, 0xE9, 0x00);     // setup gdt entry
    memset(&system_tss, 0, sizeof(system_tss));     // clear the tss

    system_tss.ss0  = 0x10;
    system_tss.esp0 = 0x0;
    system_tss.cs   = 0xb;
    system_tss.ss   = system_tss.ds = system_tss.es = system_tss.fs = system_tss.gs = 0x13;     // RPL = 3
}



void tss_swap(uint32_t stack, size_t n) {
	gdt[SEG_TSS] = SEG16(STS_T32A, &system_tss, sizeof(system_tss)-1, 0);
	gdt[SEG_TSS].s = 0;
	system_tss.ss0 = SEG_KDATA << 3;
	system_tss.esp0 = stack + n; //(uint)proc->kstack + KSTACKSIZE;
	ltr(SEG_TSS<<3);
}

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void gdt_init(void)
{

	// Map "logical" addresses to virtual addresses using identity map.
	// Cannot share a CODE descriptor for both kernel and user
	// because it would have to have DPL_USR, but the CPU forbids
	// an interrupt from CPL=0 to DPL=3.
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
	gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
	gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
	
	lgdt(gdt, sizeof(gdt));
	tss_swap(0, 0);
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