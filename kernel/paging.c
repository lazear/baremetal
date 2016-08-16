/*
paging.c
*/

#include <types.h>
#include <x86.h>
#include <paging.h>

extern void k_paging_enable();
extern void k_paging_load_directory(uint32_t* dir);
extern uint32_t* k_read_cr3();

/*
Page directory -> Page table -> Page
1024 entires in directory (1024 page tables)
1024 entires in pagetalbe (1024 pages)
4096 bytes in each page (4 KB)

= 4GB per page directory
*/

uint32_t* K_CURRENT_PAGE_DIRECTORY = 0;


void k_page_fault(struct regs* r) {
	asm volatile("cli");
	uint32_t cr2;

	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));

	printf("Page fault @ 0x%X\n", cr2);
	if (cr2 & 1) printf("\tPage Not Present\n");
	if (cr2 & 2) printf("\tPage Not Writeable\n");
	if (cr2 & 4) printf("\tPage Supervisor Mode\n");
	print_regs(r);

	asm volatile("mov %%eax, %%cr2" :: "a"(0x00000000));
	asm volatile("hlt");
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

	k_paging_load_directory(K_CURRENT_PAGE_DIRECTORY);
	k_paging_enable();
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
	//Should be no reason to reload the directory each time.
	//k_paging_load_directory(dir);
}