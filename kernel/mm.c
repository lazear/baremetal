/*
mm.c
Physical memory management - keep track of physical memory used,
this will enable the usage of a virtual memory/page mapped heap.
When the heap nears the top, sbrk() can call k_page_alloc() to get another 
0x1000 bytes (4KB) of physical address space, that can then be mapped to a
continous virtual memory segment.
*/

#include <types.h>

#define ERR_NO_MEM	-1

/*
Pointers to the current page directory bitmap
and page table bitmap
*/
uint32_t* MM_CURRENT_PD = 0;
uint32_t* MM_CURRENT_PT = 0;

// k_MM_HEAP will be set to the end of the kernel ~1mb
uint32_t* MM_HEAP = 0;
// K_MM_HEAP_MAX is 2mb. After 2mb, page_alloc is enabled.
uint32_t* MM_HEAP_MAX = 0x00200000;

/*
This function should ONLY be used to set up the initial memory manager tables.
Going to alloc some blocks right at the end of the kernel

Memory allocated by this function will NEVER be freed.
*/
uint32_t* mm_alloc(size_t n) {
	if (MM_HEAP == 0)
		return NULL;

	if (MM_HEAP + n < MM_HEAP_MAX) {
		uint32_t* addr = MM_HEAP;
		if (n >= 0x1000)
			MM_HEAP += (n & ~0xFFF);		// Make sure we stay 0x1000 aligned.
		else
			MM_HEAP += 0x1000;			// Even if n is small, increment by 0x1000
		return addr;
	} else
		return NULL;
}


/*
Bitmap is an array of 32 * 32 bit integers.
Setting a bit indicates that it is in use
*/
void mm_bitmap_set_bit(uint32_t* b, uint32_t bit) {
	b[bit/32] |= (0x1 << (bit % 32));
}

/*
Bitmap is an array of 32 * 32 bit integers.
Unsetting a bit indicates that it is free
*/
void mm_bitmap_unset_bit(uint32_t* b, uint32_t bit) {
	b[bit/32] &=  ~(0x1 << (bit % 32));
}

/*
Returns the first bit position (bit number in entry ( 32 * array index))
that is 0 (free)
*/
int mm_first_free(uint32_t* b) {
	for (int q = 0; q < 32; q++) {
		uint32_t free_bits = (b[q] ^ ~0x0);
		for (int i = 0; i < 32; i++ ) 
			if (free_bits & (0x1 << i)) 
				return i + (q*32);
	}
	return ERR_NO_MEM;
}

/*
Allocate a bitmap (1024 bits, 128 bytes)
Set every entry to 0 (free)
*/
uint32_t* mm_bitmap_init() {
	size_t bitmap_size = sizeof(uint32_t) * 32;
	uint32_t* b = mm_alloc(bitmap_size);
	memset(b, 0, bitmap_size);
	return b;
}

/*
Align the virtual address to the page directory and page table bitmaps,
then mark it as free
*/
uint32_t* mm_page_free(uint32_t* PD, uint32_t* PT, uint32_t* address) {
	uint32_t aligned = (uint32_t) address & ~0x3FF;

	uint32_t pd_index = (uint32_t) aligned >> 22;
	uint32_t pt_index = (uint32_t) aligned >> 12 & 0x3ff;

	mm_bitmap_unset_bit(PT, pt_index);

	/*	
	If the PD that this was in was full, mark it as not full now
	DON't do this, as stated below. We need an array of pointers
	Holding the location of the last page table.
	*/
	/*
	if (mm_first_free(PD) != pd_index) {
		kprintx("PAGEFREE:", pd_index);
		mm_bitmap_unset_bit(PD, pd_index);
	}
	*/

	return address;
} 


/*
Initial implementation will be allocating one page.
Returns the address of the page
Marks the page as used (bit shift 1)
Returns ERR_NO_MEM (-1) if page directory is full.
*/
uint32_t* mm_page_alloc(uint32_t* PD, uint32_t* PT) {
	uint32_t ff_PD = mm_first_free(PD);
	uint32_t ff_PT = mm_first_free(PT);

	if (ff_PD != ERR_NO_MEM && ff_PT != ERR_NO_MEM) {
		uint32_t address = ((ff_PD * 0x1000 * 0x400) + (ff_PT * 0x1000));

		mm_bitmap_set_bit(PT, ff_PT);	// Mark the page as no longer free
		return address;

	} else if (ff_PD != ERR_NO_MEM && ff_PT == ERR_NO_MEM) {
		/*
		PT is now full, so mark the PD as full also
		Set the bit of the PD as the entire page being full
		Which means we should change the pointer to PT to a new page table
		since it's full, which means we don't need to save the data for it.
		*/
		/*
		ACTUALLY DON'T.... This allows the entire page table to be reset under 
		the following circumstances:
		If the page table is completely full (which it is if we are here), we 
		want to delete that entry. But if we then free one of those pages, we
		can unmark the PD bit for that table - but we no longer have a bitmap
		with the memory layout for that page table. Maybe we should keep a list of 
		pointers? Or doing a bitmap for the page directory is a bad idea... but
		the other option is doing uint32 pd[1024], which takes up a whole 4KB of memory

		So working solution for now is to just not unmark a directory after it's been marked full
		So pages can continue to be used and free()d until the entire page table is full.
		Then we switch to the next one.
		*/
		PT = mm_bitmap_init();
		

		vga_pretty("\nSwapping page table bitmap\n", 0x05);

		MM_CURRENT_PT = PT;
		mm_bitmap_set_bit(PD, ff_PD);
		// Now recursively try again

		mm_page_alloc(PD, PT);


	} else {
		// This means both the PT and the PD are full
		return ERR_NO_MEM;
	}
}



/* 
Wrapper for mm_page_alloc using global pointers
Concept is to have malloc/whatever the virtual memory manager is call
k_page_alloc() to get the heap, and then once that heap is outgrown, call alloc again.
Once the entire old heap is free()d, free the page.
*/
uint32_t* k_page_alloc() {
	return mm_page_alloc(MM_CURRENT_PD, MM_CURRENT_PT);
}

uint32_t* k_page_free(uint32_t* addr) {
	return mm_page_free(MM_CURRENT_PD, MM_CURRENT_PT, addr);
}

void mm_debug() {
	printf("Physical Memory Management Debug:\n");
	int ptidx = mm_first_free(MM_CURRENT_PT)/32;
	int pdidx = mm_first_free(MM_CURRENT_PD)/32;
	printf("PT (%d): %b\n", ptidx, MM_CURRENT_PT[ptidx]);
	printf("PD (%d): %b\n", pdidx, MM_CURRENT_PD[pdidx]);
}


/*
8 bits per byte * 4 bytes * 32 = 128 bytes (1024 bits)
Each bit represents whether one 4096 byte page has been allocated or not.
This means that one 128 byte sized bitmap holds information for 4 MB of data
One bitmap[32] represents one page table entry, of which there are 1024 in a 
page directory;
Each bitmap[i] entry represents 1024 pages.
*/

/*
We're going to return a one-time use address for setting up virtual paging.
It'll be entry 1023 in the first page table in the first Page directory entry
That entry will then be set to full.

Initialize the page heap; and then allocate the first two new bitmaps.
*/
void* k_mm_init(uint32_t heap) {

	heap = (heap + 0x1000) & ~0xFFF;
	MM_HEAP = heap;
//	kprintx("Heap@: ", heap);
	MM_CURRENT_PD = mm_bitmap_init();
	MM_CURRENT_PT = mm_bitmap_init();


	/*First 2 mb are reserved as used.
	~1Mb to 2Mb are for the K_MM_HEAP, but that shouldn't use more than a couple kb
	This allows us the benefit of calling k_page_alloc for actual paging.

	TODO: Change this to actual end-of-kernel space, so everything can be in agreement.
	*/
	for (int i = 0; i < 512; i++)
		mm_bitmap_set_bit(MM_CURRENT_PT, i);	

	uint32_t* addr = mm_page_alloc(MM_CURRENT_PD, MM_CURRENT_PT);

	//kprintx("Initial addr:", addr);
	//kprintb("PT bm:", MM_CURRENT_PT[31]);
	return addr;
}


/*
Function to play around with and test the physical mem manager.
*/
void mm_test() {
	// each bit in the pdb represents 4mb of address space
	uint32_t ff_pdb = mm_first_free(MM_CURRENT_PD);
	uint32_t ff_ptb = mm_first_free(MM_CURRENT_PT);

	printf("PD @ %x | PT @ %x\n", MM_CURRENT_PD, MM_CURRENT_PT);
	printf("ff_pdb %d\n", ff_pdb);
	printf("ff_ptb %d\n", ff_ptb);

	uint32_t first_address = (ff_pdb* 0x1000 * 0x400) + (ff_ptb * 0x1000);

	for (int i = 0; i < 5; i++ ) {
		int result = k_page_alloc();
		if (result == ERR_NO_MEM)
			vga_pretty("[FAIL] no memory left", 0x4);
		else {
			printf("Testing allocation: 0x%x : PT$ %b\n", result, MM_CURRENT_PT[mm_first_free(MM_CURRENT_PT)/32]);
		}
	}


	
	k_page_free(first_address + 0x30);
	printf("Testing free offset: 0x%x :PT$ %b\n", first_address, MM_CURRENT_PT[mm_first_free(MM_CURRENT_PT)/32]);
	uint32_t* next = k_page_alloc();

	if (next == first_address)
		vga_pretty("Success!\n", 0x0A);
	else {
		vga_pretty("Something went wrong\n", 0x4);
		printf("Current ff_pdb: 0x%x\n", ff_pdb);
		printf("Current bitmap value: %b\n", MM_CURRENT_PD[1]);
	}

	for (int i = 0; i < 3; i++ ) {
		int result = k_page_alloc();
		if (result == ERR_NO_MEM)
			vga_pretty("[FAIL] no memory left", 0x4);
		else {
			printf("Testing allocation: 0x%x\n", result);
		}
	}

}