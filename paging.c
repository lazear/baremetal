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

uint32_t page_directory[1024] __attribute__((aligned(4096)));

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

/*
Remap a virtual address to a physical address
This will be more useful in user mode/multithreading environment

We need to lookup the page for a given virtual address, 
and then set it to the physical address + flags
*/

void flush_tlb(uint32_t mem)
{
	asm volatile("invlpg %0" :: "m"(mem));
}


uint32_t* k_unmap(uint32_t virt) {
	uint32_t page_directory_index = virt >> 22;
	uint32_t page_table_index = virt >> 12 & 0x03FF;	// Shift 12 AND last 12 bits

	uint32_t* page_table = (uint32_t*) page_directory[page_directory_index];
	//uint32_t* page_table = (uint32_t*) 0xFFC00000 + (0x400 * page_directory_index);

	return (void*)((page_table[page_table_index] & ~0xFFF) + ((uint32_t) virt & 0xFFF));
}

void k_map(uint32_t phys, uint32_t virt, uint8_t flags) {
	uint32_t page_directory_index = virt >> 22;
	uint32_t page_table_index = virt >> 12 & 0x03FF;	// Shift 12 AND last 12 bits

	//uint32_t* page_table = page_directory[page_directory_index];
	uint32_t* page_table = 	(uint32_t*) 0xFFC00000 + (0x400 * page_directory_index);
	uint32_t* pd = 			(uint32_t*) 0xFFF00000;

	pd[page_directory_index] |= 0x3;	// Mark as Present, RW
	page_table[page_table_index] = phys | (flags & 0xFFF) | PF_PRESENT;

	load_page_directory(pd);
	flush_tlb(virt);

}


void k_page_fault(struct regs* r) {
	asm volatile("cli");
	uint32_t cr2;
	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));
	kprintx("Page fault occured at: ", cr2);
	for(;;);
}

uint32_t* k_paging_get_dir(){
	return page_directory;
}

uint32_t* k_paging_get_phys(uint32_t* dir, uint32_t virt) {
	int _pdi = virt >> 22;
	int _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt = (uint32_t*) dir[_pdi];

	return ((uint32_t*) pt[_pti]);
}

void k_paging_map_block( uint32_t* dir, uint32_t phys, uint32_t virt, uint8_t flags) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt;

	int v = dir[_pdi];

	kprintx("Dir val", v);
	if (v == 0x2) {
		uint32_t* pt =   page_alloc();
		memset(pt, 0x3, 0x1000);
		kprintx("Alloced: ", pt);
	} else {
		//uint32_t* pt = v & ~0x3ff;
	}

	pt[_pti] = (phys) | 0x3;

	//dir[_pdi] = ((uint32_t)pt | PF_PRESENT | PF_RW);
	dir[_pdi] = (0x00300000 + k_p_index()-0x1000| 0x3);

	kprintx("Page Dir Value:   ", dir[_pdi]);
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

	load_page_directory(dir);

}

void k_paging_init() {

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

	for (int i = 0; i < 1024; i++) {
		// 0x1000 = 4 KB * 1024 = 4 MB
		first_page_table[i] = (i * 0x1000) | PF_PRESENT | PF_RW | PF_SUPERVISOR; 

	}

	// Map out the first 4 MB for the kernel
	page_directory[0] =		((uint32_t) first_page_table | 3);

	// Map the last 4 MB to the page directory
	page_directory[1023] =	((uint32_t) page_directory | 0x3);
	kprintx("FPT: ", page_directory[10]);
	load_page_directory(&page_directory);
	paging_enable();

}