/*
paging.c


Linear address structure - 32 bits of address space w/ 4KB pages
31			 22	21	 	  12 11		0
-------------------------------------
| Directory 	| Table 	| Offset|
-------------------------------------

*/

#include <types.h>
#include <x86.h>
#include <paging.h>



extern void paging_enable();
extern void load_page_directory(uint32_t* dir);

// Page directory -> Page table -> Page
uint32_t page_directory[1024] __attribute__((aligned(4096)));

uint32_t* paged_memory_start  = 0;
uint32_t* paged_memory_end = 0;


uint32_t switch_pd(uint32_t dir)
{
	uint32_t cr3;
	asm volatile("mov %%cr3, %%eax" : "=a"(cr3));
	asm volatile("mov %%eax, %%cr3" :: "a"(dir));
	return cr3;
}

void enable_paging(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %%eax" : "=a"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %%eax, %%cr0" :: "a"(cr0));
}

void map(uint32_t pd, uint32_t phys, uint32_t virt, uint8_t flags) {
	uint8_t id = virt >> 22;
	kprintx("", id);
}


void k_page_fault(struct regs* r) {
	asm volatile("cli");
	uint32_t cr2;
	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));
	kprintx("Page fault occured at: ", cr2);
	for(;;);
}

void paging_init() {

	/* load a custom ISR14 handler for page faults */
	idt_set_gate(14, k_page_fault, 0x08, 0x8E);

	for (int i =0; i < 1024; i++) 
		page_directory[i] = PF_RW;

	/* addressed to 0x00004000
	/ first page is 	0x00000000 | 3
	/ last page is 	0x003FF000 | 3
	/ initialize a 4MB page directory
	/ page_table[num] >> 12 == num for linear mapping
	*/
	uint32_t first_page_table[1024] __attribute__((aligned(4096)));
	uint32_t second_page_table[1024]__attribute__((aligned(4096)));
	for (int i = 0; i < 1024; i++) {
		// 0x1000 = 4 KB * 1024 = 4 MB
		first_page_table[i] = (i * 0x1000) | PF_PRESENT | PF_RW | PF_SUPERVISOR; 

	}
	for (int i = 0; i < 1024; i++) {
		// 0x1000 = 4 KB * 1024 = 4 MB
		second_page_table[i] = (i * 0x1000) + 0x00400000| PF_PRESENT | PF_RW | PF_SUPERVISOR; 

	}

	page_directory[0] = ((uint32_t) first_page_table) | 3;
	page_directory[1] = ((uint32_t) second_page_table) | 3;
	

	load_page_directory(&page_directory);
	paging_enable();

	char* buf = alloc(64);
	itoa(first_page_table, buf, 2);
	kprintx("PT: ", buf);

	//vga_pretty(buf, 0x2);
	//enable_paging();
}