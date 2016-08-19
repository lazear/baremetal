/*
paging.c
*/

#include <types.h>
#include <x86.h>
#include <paging.h>


/*
Page directory -> Page table -> Page
1024 entires in directory (1024 page tables)
1024 entires in pagetalbe (1024 pages)
4096 bytes in each page (4 KB)

= 4GB per page directory
*/

uint32_t* KERNEL_PAGE_DIRECTORY = 0;
uint32_t* CURRENT_PAGE_DIRECTORY = 0;

void k_swap_pd(uint32_t* pd) {
	CURRENT_PAGE_DIRECTORY = pd;
	k_paging_load_directory(pd);
}


/* We are going to try and fix PF's by increasing heap if that's the issue */
void k_page_fault(struct regs* r) {
	pushcli();
	uint32_t cr2;

	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));

	printf("Page fault @ 0x%X\n", cr2);
	printf("Phys: %x\n", k_paging_get_phys(cr2));
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

uint8_t _paging_get_phys(uint32_t* dir, uint32_t virt) {
	int _pdi = virt >> 22;
	int _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt = dir[_pdi] & ~0x3FF;
	return ((uint32_t*) pt[_pti]);
}

int _paging_unmap(uint32_t* dir, uint32_t virt) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;
	
	uint32_t* pt;

	if ( dir[_pdi] & ~0x3FF )
		pt = dir[_pdi] & ~0x3FF;
	else 
		return 0;	// This address was never mapped in the first place
	pt[_pti] = 0;	// Reset all flags.

	//k_paging_load_directory(CURRENT_PAGE_DIRECTORY);
	// Invalidate the old page.
	asm volatile("invlpg (%0);" : : "b"(virt) : "memory");
	return 1;
}

void _paging_map(uint32_t* dir, uint32_t phys, uint32_t virt, uint8_t flags) {
	uint32_t _pdi = virt >> 22;
	uint32_t _pti = (virt >> 12) & 0x3FF;

	uint32_t* pt;

	if ( dir[_pdi] & ~0x3FF ) {
		pt = dir[_pdi] & ~0x3FF;
	} else {
	//	vga_puts("SHOULD NOT SEE THIS");
		pt = k_page_alloc();
	}

	pt[_pti] = ((phys) | flags | PF_PRESENT );
	dir[_pdi] = ((uint32_t) pt | flags | PF_PRESENT);
}

uint32_t* k_paging_get_phys(uint32_t virt) {
	return _paging_get_phys(CURRENT_PAGE_DIRECTORY, virt);
}

void k_paging_init(uint32_t* dir_addr) {
	/* load a custom ISR14 handler for page faults */
	idt_set_gate(14, k_page_fault, 0x08, 0x8E);

	CURRENT_PAGE_DIRECTORY = dir_addr;
	memset(CURRENT_PAGE_DIRECTORY, 0, 4096);
	//uint32_t* table = k_heap_alloca(4096);

	uint32_t* table = k_page_alloc();
	
	for (int i = 0; i < 1024; i++) {
		table[i] = (i * 0x1000) | 0x3;
	}


	CURRENT_PAGE_DIRECTORY[0] = (uint32_t) table | 3;
	CURRENT_PAGE_DIRECTORY[1023] = (uint32_t) CURRENT_PAGE_DIRECTORY | 3;
	KERNEL_PAGE_DIRECTORY = CURRENT_PAGE_DIRECTORY;
	k_paging_load_directory(CURRENT_PAGE_DIRECTORY);
	k_paging_enable();
}

/* 
For some reason, k_paging_map seems to work
without reloading the PD into CR3, however,
unmapping a page requires remapping
*/
void k_paging_map(uint32_t phys, uint32_t virt, uint8_t flags) {
	_paging_map(CURRENT_PAGE_DIRECTORY, phys, virt, flags);
}

int k_paging_unmap(uint32_t virt) {
	return _paging_unmap(CURRENT_PAGE_DIRECTORY, virt);
}

uint32_t* k_copy_pagedir(uint32_t* dest, uint32_t* src) {
	for (int i = 0; i < 1024; i++) {
		if (src[i] & ~0x3FF) {
			uint32_t* dest_pt;
			uint32_t* src_pt = src[i]  & ~0x3FF;

			if (dest[i] & ~0x3FF)
				dest_pt = dest[i] & ~0x3FF;
			else 
				dest_pt = k_page_alloc(); 
			
			for (int q = 0; q < 1024; q++) {
				if (src_pt[q] & ~0x3FF) {
					dest_pt[q] = src_pt[q];
				}
			}
			dest[i] = (uint32_t) dest_pt | PF_PRESENT;
		}
	}
	return src;
}

void walk_pagedir(uint32_t* pd) {
	for (int i = 0; i < 1024; i++) {
		if (pd[i] & ~0x3FF) {
			uint32_t* pt = pd[i]  & ~0x3FF;
			for (int q = 0; q < 1024; q++) {
				if (pt[q] & ~0x3FF)
					printf("%x -> %x\n", pt[q], ( (i << 22) + (q << 12)));
			}
		}
	}
	printf("pd @ 0x%x\n", pd);
}


void _paging_map_more(uint32_t* pd, uint32_t virt, uint32_t numpages, int flags) {
	if (!pd) {
		panic("NULL page directory");
		return;
	}

	if (numpages > 1024) {
		uint32_t page_tables_needed = numpages / 1024 + 1;

		for (int i = 0; i < page_tables_needed; i++) {


			uint32_t _pdi = ((virt + (i * 0x1000 * 0x400)) >> 22);
			if (!(pd[i] & ~0x3FF)) {
				uint32_t* pt = k_page_alloc();
				pd[_pdi] = ((uint32_t) pt | flags );
			}
		}
	}
	for (int i = 0; i < numpages; i++) {
		uint32_t phys = k_page_alloc();
		if (!phys) panic("MM error");
		_paging_map(pd, phys, virt + (i* 0x1000), flags);
	}
}

uint32_t* k_create_pagedir(uint32_t virt, uint32_t numpages, int flags) {
	uint32_t* pd = k_page_alloc();		// 0xFFFF0000 virtual address
	//uint32_t* table = k_page_alloc();	// 0xFFC00000 virtual address
	memset(pd, 0, 0x1000);
	//memset(table, 0, 0x1000);

	pd[1023] = (uint32_t) pd | flags;

	uint32_t phys = 0;

	/*
	I think I've sorted out what the page fault issue is. The page 
	tables are being allocated as we go, which means that PT 1025 is 
	called after the first 1024 k_page_alloc() calls, placing it above 
	4mb of memory. It would be more wise to calculate the number of needed
	page tables from numpages and virt and then allocate them FIRST. Then
	we don't need to map out a ton of pages in the kernel pd.
	*/

	if (numpages > 1024) {
		uint32_t page_tables_needed = numpages / 1024 + 1;

		for (int i = 0; i < page_tables_needed; i++) {

			uint32_t _pdi = ((virt + (i * 0x1000 * 0x400)) >> 22);
			printf("%d pdi\n", _pdi);
			uint32_t* pt = k_page_alloc();
			
			pd[_pdi] = ((uint32_t) pt | flags );
			printf("%x\n", pd[_pdi]);

		}
	}

	for (int i = 0; i < numpages; i++) {
		phys = k_page_alloc();
		if (!phys) 
			panic("MM error");
		_paging_map(pd, phys, virt + (i* 0x1000), flags);
	}

	return pd;
}