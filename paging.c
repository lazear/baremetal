/*
paging.c
*/

#include <types.h>
#include <x86.h>
#include <paging.h>

extern void paging_enable();
extern void load_page_directory(uint32_t* dir);

/*
Page directory -> Page table -> Page
1024 entires in directory (1024 page tables)
1024 entires in pagetalbe (1024 pages)
4096 bytes in each page (4 KB)

= 4GB per page directory
*/


uint32_t* K_CURRENT_PAGE_DIRECTORY = 0;

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

void flush_tlb(uint32_t mem)
{
	asm volatile("invlpg %0" :: "m"(mem));
}

void k_page_fault(struct regs* r) {
	asm volatile("cli");
	uint32_t cr2;
	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));
	kprintx("Page fault occured at: ", cr2);
	for(;;);
}

uint32_t* k_paging_get_phys(uint32_t* dir, uint32_t virt) {
	int _pdi = virt >> 22;
	int _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt = (uint32_t*) dir[_pdi];

	return ((uint32_t*) pt[_pti]);
}

void k_paging_init(uint32_t* dir_addr) {
	/* load a custom ISR14 handler for page faults */
	idt_set_gate(14, k_page_fault, 0x08, 0x8E);

	K_CURRENT_PAGE_DIRECTORY = dir_addr;
	memset(K_CURRENT_PAGE_DIRECTORY, 0, 4096);
	//uint32_t* table = k_heap_alloca(4096);

	uint32_t* table = k_page_alloc();
	
	for (int i = 0; i < 1024; i++) {
		table[i] = (i * 0x1000) | 0x3;
	}


	K_CURRENT_PAGE_DIRECTORY[0] = (uint32_t) table | 3;
	K_CURRENT_PAGE_DIRECTORY[1023] = (uint32_t) K_CURRENT_PAGE_DIRECTORY | 3;

	load_page_directory(K_CURRENT_PAGE_DIRECTORY);
	paging_enable();
}

void k_paging_map( uint32_t phys, uint32_t virt, uint8_t flags) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;
	uint32_t* dir = K_CURRENT_PAGE_DIRECTORY;
	
	uint32_t* pt;

	if ( dir[_pdi] & ~0x3FF ) {
		pt = dir[_pdi] & ~0x3FF;
	} else {
		//vga_puts("Hmmm...\n");
		pt = k_page_alloc();
	}


	pt[_pti] = ((phys) | flags | PF_PRESENT);

	dir[_pdi] = ((uint32_t) pt | flags | PF_PRESENT);
/*	kprintx("Page Dir Value:   ", dir[_pdi]);
	kprintx("Page Dir Index:   ", _pdi);
	kprintx("Page Table Value: ", pt[_pti]);
	kprintx("Page Table Index: ", _pti);
	kprintx("Virtual Address:  ", virt);
	kprintx("Physical Address: ", phys);

	// Each page table  (directory entry) represents 4MB (0x400 * 0x1000 * 0x400) of address space
	// Each page entry (table index) represents 
	kprintx("Begin dir:",  (_pdi * 0x400 * 0x1000));
	kprintx("Begin tab: ", (_pdi * 0x400 * 0x1000) + (_pti * 0x1000));
	kprintx("End tab: ", (_pdi * 0x400 * 0x1000) + (_pti* 0x1000 + 0x1000));
	kprintx("End dir: ", (_pdi * 0x400 * 0x1000) + (0x400 * 0x1000 + 0x1000));
*/
	load_page_directory(dir);
}