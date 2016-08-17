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

/* We are going to try and fix PF's by increasing heap if that's the issue */
void k_page_fault(struct regs* r) {
	pushcli();
	uint32_t cr2;

	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));

	printf("Page fault @ 0x%X\n", cr2);
	if (cr2 & 1) printf("\tPage Not Present\n");
	if (cr2 & 2) printf("\tPage Not Writeable\n");
	if (cr2 & 4) printf("\tPage Supervisor Mode\n");

	if (cr2 >= heap_brk() && cr2 < heap_brk() + 0x1000){
		printf("Heap is out of memory\n");
		sbrk(0x1000);
		popcli();
		return;
	}

	printf("Calling func: %x\n", r->eip);
	//print_regs(r);
	//traverse_blockchain();
	mm_debug();

	asm volatile("mov %%eax, %%cr2" :: "a"(0x00000000));
	asm volatile("hlt");
}



uint32_t* k_paging_get_phys(uint32_t virt) {
	uint32_t* dir = K_CURRENT_PAGE_DIRECTORY;
	int _pdi = virt >> 22;
	int _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt = dir[_pdi] & ~0x3FF;
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

/* 
For some reason, k_paging_map seems to work
without reloading the PD into CR3, however,
unmapping a page requires remapping
*/
void k_paging_map(uint32_t phys, uint32_t virt, uint8_t flags) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;
	uint32_t* dir = K_CURRENT_PAGE_DIRECTORY;
	
	uint32_t* pt;

	if ( dir[_pdi] & ~0x3FF ) {
		pt = dir[_pdi] & ~0x3FF;
	} else {
		pt = k_page_alloc();
	}


	pt[_pti] = ((phys) | flags | PF_PRESENT);

	dir[_pdi] = ((uint32_t) pt | flags | PF_PRESENT);
}

int k_paging_unmap(uint32_t virt) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;
	uint32_t* dir = K_CURRENT_PAGE_DIRECTORY;
	
	uint32_t* pt;

	if ( dir[_pdi] & ~0x3FF )
		pt = dir[_pdi] & ~0x3FF;
	else 
		return 0;	// This address was never mapped in the first place
	pt[_pti] = 0;	// Reset all flags.

	//k_paging_load_directory(K_CURRENT_PAGE_DIRECTORY);
	// Invalidate the old page.
	asm volatile("invlpg (%0);" : : "b"(virt) : "memory");
	return 1;
}