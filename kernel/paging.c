/*
paging.c

WARNING: Page tables and directories are currently being mapped out by mm_alloc
as a shortcut. This can only support ~240 such calls. Need to implement a way 
to k_page_alloc() them and then map those pages to the current page directory.


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
void k_page_fault(regs_t * r) {
	pushcli();
	uint32_t cr2;

	asm volatile("mov %%cr2, %%eax" : "=a"(cr2));

	printf("Page fault @ 0x%X\n", cr2);
	printf("Phys: %x\n", k_virt_to_phys(cr2));
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

uint32_t* _virt_to_phys(uint32_t* dir, uint32_t virt) {
	int _pdi = virt >> 22;
	int _pti = (virt >> 12) & 0x3FF;

	if (!dir[_pdi] & ~0x3FF)
		return NULL;

	uint32_t* pt = dir[_pdi] & ~0x3FF;

	return ((uint32_t*) pt[_pti]);
}

/* 
Is a physical address mapped in the PD? 
Returns the virtual address if mapped, 0 if not
*/
int _phys_to_virt(uint32_t* pd, uint32_t phys) {
	for (int i = 0; i < 1024; i++) {
		if (pd[i] & ~0x3FF) {
			uint32_t* pt = pd[i]  & ~0x3FF;
			for (int q = 0; q < 1024; q++) {
				if (pt[q] & ~0x3FF)
					if ((pt[q] & ~0x3FF) == (phys & ~0x3FF)) 
						return  ((i << 22) + (q << 12));
			}
		}
	}
	return 0;
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
		pt = mm_alloc(0x1000);//k_page_alloc();
	}

	pt[_pti] = ((phys) | flags | PF_PRESENT );
	dir[_pdi] = ((uint32_t) pt | flags | PF_PRESENT);
}

uint32_t* k_virt_to_phys(uint32_t virt) {
	return _virt_to_phys(CURRENT_PAGE_DIRECTORY, virt);
}

uint32_t* k_phys_to_virt(uint32_t phys) {
	return _phys_to_virt(CURRENT_PAGE_DIRECTORY, phys);
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

/*
Map many continous pages (numpages) starting at virtual address (virt) to a
page directory
*/
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
				uint32_t* pt = mm_alloc(0x1000);//k_page_alloc();
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


void free_pagedir(uint32_t* pd) {
	for (int i = 0; i < 1024; i++) {
		if (pd[i] & ~0x3FF) {
			uint32_t* pt = pd[i]  & ~0x3FF;
			for (int q = 0; q < 1024; q++) {
				if (pt[q] & ~0x3FF) {
					k_page_free(pt[q] & ~0x3FF);
					//_paging_unmap(pd, ( (i << 22) + (q << 12)));
				}
			}
		}
	}
}

/*
Copy n bytes starting at virtual address virt from the physical address of virt
in the source page directory to the physical address of virt in the destination
page directory. Basically so that two address spaces will contain the same
virtual addresses and data but reside at different physical memory locations.

Primary use of this function will be fork(). Could probablyvwrite a separate 
function that is like memcpy.

Virt may not be page aligned. Need to take into consideration this, in addition
to the fact that the physical memory space is probably not contigous. 

Also, the physical addresses we need to access may not be mapped in the current
page directory, so we need to check that.
*/
void page_copy(uint32_t* dest, uint32_t* src, uint32_t virt, size_t n) {

	uint32_t va0 = (virt & ~0xFFF);		// Round down to the first page.
	uint32_t off = ((uint32_t) virt - (uint32_t) va0);

	size_t sz = n;
	for (int i = 0; i < n; i+= 0x1000) {
		va0 += 0x1000;
		size_t cp = (sz >= 0x1000) ? 0x1000 - off : sz;


	//	uint32_t _pdi = va0>> 22;
	//	uint32_t _pti = (va0 >> 12) & 0x3FF;

		bool src_free = false;
		bool dest_free = false;
		uint32_t pa_src = _virt_to_phys(src, va0);
		uint32_t pa_dest = _virt_to_phys(dest, va0);


		if (!pa_src) {
			printf("%x ", va0);
			panic("Page not mapped in source page directory!");
			return;
		}
		if (!pa_dest) {
			/* We can fix an error on the destination side by just mapping 
			those pages, and then recursively calling page_copy */
			panic("Missing pages in dest");
			return;
		}	

		/* This is where things might get confusing...
		We translated the virtual address to physical addresses for the source
		and destination, now we need to make sure those physical addresses
		are mapped in whatever the current page directory is. If they aren't 
		mapped, we need to temporarily map them */
		uint32_t va_src = (uint32_t) k_phys_to_virt(pa_src);
		uint32_t va_dest = (uint32_t) k_phys_to_virt(pa_dest);

		if (!va_src) {
			// Physical address of the source is not mapped in the current PD
			va_src = malloc_a(0x1000);
			k_paging_map(pa_src, va_src, 0x7);
			//printf("src: Had to malloc_a %x\n", va_src);
			src_free = true;

		} 
		if (!va_dest) {
			// physical address of the destination is not mapped in current PD.
			va_dest = malloc_a(0x1000);
			k_paging_map(pa_dest, va_dest, 0x7);		// map in the temporary
			//printf("Mapping phys %x to virt %x in CURRENT_PD \n", pa_dest, va_dest);
			dest_free = true;
			// should unmap this, but does it matter?
		} 

		memcpy(va_dest + off, va_src + off, cp);

		sz -= cp;
		off = 0;		// Eliminate the offset after first page
		if (dest_free) {
			free(va_dest);
			k_paging_unmap(va_dest);
		}
		if (src_free) {
			k_paging_unmap(va_src);
			free(va_src);
		}
	}	// copy one page at a time

		
}

/* 
Map until end of kernel, no pages allocated. Scratch that, map until 2Mb. 
End of kernel was not working, likely because there is important information 
stored directly after the kernel (bitmaps, stack, etc)
*/
void k_map_kernel(uint32_t* pd) {
	for (int i = 0; i < 0x00200000; i+=0x1000)
		_paging_map(pd, i, i, 0x7);
}

uint32_t* k_create_pagedir(uint32_t virt, uint32_t numpages, int flags) {
	uint32_t* pd = mm_alloc(0x1000);	// 0xFFFF0000 virtual address
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
			uint32_t* pt = mm_alloc(0x1000);//k_page_alloc();
			
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