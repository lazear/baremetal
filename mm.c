/*

mm.c
Physical memory management

*/

#include <types.h>

uint32_t PAGE_FRAME_BEGIN = 0;

#define ERR_NO_MEM	-1

struct  KHEAPBLOCK {
	struct KHEAPBLOCK	*next;
	uint32_t	size;
	uint8_t		used;
} __attribute__((__packed__));

struct  KHEAPBM {
	struct KHEAPBLOCK 	*block;
} __attribute__((__packed__));

// QUICK and DIRTY alloc function

uint32_t* HEAP = 0x00200000;

int INDEX = 0;
int PINDEX = 0;
void *alloc(size_t bytes) {
	
	void* ptr = HEAP + INDEX;
	INDEX+=bytes;
	return ptr;

}

void* page_alloc() {
	void* ptr = HEAP + (PINDEX * 0x1000);
	PINDEX++;
	return ptr;
}

void* free(void* ptr) {
	return ptr;
}

int k_p_index() {
	return PINDEX * 0x1000;
}

void k_heap_init(uint32_t n) {
	PAGE_FRAME_BEGIN = n;
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
	uint32_t* b = alloc(bitmap_size);
	memset(b, 0, bitmap_size);
	kprintd("size", bitmap_size);
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
} 


/*
Initial implementation will be allocating one page.
*/
uint32_t* mm_page_alloc(uint32_t* PD, uint32_t* PT) {
	uint32_t ff_PD = mm_first_free(PD);
	uint32_t ff_PT = mm_first_free(PT);

	if (ff_PD != ERR_NO_MEM && ff_PT != ERR_NO_MEM) {
		uint32_t address = ((ff_PD * 0x1000 * 0x400) + (ff_PT * 0x1000));

		mm_bitmap_set_bit(PT, ff_PT);	// Mark the page as no longer free

		return address;

	} else if (ff_PD != ERR_NO_MEM && ff_PT == ERR_NO_MEM) {
		// PT is now full, so mark the PD as full also
		mm_bitmap_set_bit(PD, ff_PD);
		// Now recursively try again
		mm_page_alloc(PD, PT);

	} else {
		return ERR_NO_MEM;
	}
}

/*
8 bits per byte * 4 bytes * 32 = 128 bytes (1024 bits)
Each bit represents whether one 4096 byte page has been allocated or not.
This means that one 128 byte sized bitmap holds information for 4 MB of data
One bitmap[32] represents one page table entry, of which there are 1024 in a 
page directory;
Each bitmap[i] entry represents 1024 pages.
*/

void mm_test() {

	// each bit in the pdb represents 4mb of address space
	uint32_t* page_directory_bitmap = mm_bitmap_init();

	mm_bitmap_set_bit(page_directory_bitmap, 0);	// First 4MB are reserved for kernel

	uint32_t ff_pdb = mm_first_free(page_directory_bitmap);

	// each bit in the ptb represents 4kb of address space
	uint32_t* b = mm_bitmap_init();

	uint32_t ff_ptb = mm_first_free(b);
	kprintx("First free address: ", (ff_pdb* 0x1000 * 0x400) + (ff_ptb * 0x1000));

	for (int i = 0; i < 6; i++ )
		kprintx("Palloc test: ", mm_page_alloc(page_directory_bitmap, b));
	mm_page_free(page_directory_bitmap, b, mm_page_alloc(page_directory_bitmap, b) + 0x30);
}