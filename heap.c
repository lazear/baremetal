/*
Heap functions
malloc()
free()
sbrk()
brk()
*/

#include <types.h>

uint32_t PAGE_FRAME_BEGIN = 0;

struct  KHEAPBLOCK {
	struct KHEAPBLOCK	*next;
	uint32_t	size;
	uint8_t		used;
} __attribute__((__packed__));

struct  KHEAPBM {
	struct KHEAPBLOCK 	*block;
} __attribute__((__packed__));

// QUICK and DIRTY alloc function

const uint32_t* K_HEAP_BOTTOM = 0xC0000000;
const uint32_t K_HEAP_MAX = 0xF0000000;

uint32_t K_HEAP_TOP = 0;

int K_HEAP_INDEX = 0;

int INDEX = 0;
uint32_t LEGACY_HEAP = 0x00300000;
void *alloc(size_t bytes) {
	
	void* ptr = LEGACY_HEAP + INDEX;
	INDEX+=bytes;
	return ptr;

}


void* free(void* ptr) {
	return ptr;
}


uint32_t k_heap_top() {
	return K_HEAP_TOP;
}


/*
Rough implementation/estimation of sbrk. It's going to increase the size 
by 0x1000 only for right now. 
*/
void* sbrk(size_t n) {
	if (!n)
		return NULL;


	int num_of_blocks = 1;
	if (n > 0x1000)
		num_of_blocks = (n/0x1000);

	for (int i = 0; i < num_of_blocks; i++) {
		uint32_t* phys = k_page_alloc();
		k_paging_map(phys, K_HEAP_TOP, 0x3);
		K_HEAP_TOP += 0x1000;
	}
	return K_HEAP_TOP;
}


// Waterfall malloc
void* wf_malloc(size_t n) {
	uint32_t heap_sz =  (uint32_t) K_HEAP_BOTTOM + K_HEAP_INDEX;
	if ( n + heap_sz > K_HEAP_TOP) {
		uint32_t exceed = (n + heap_sz) - K_HEAP_TOP;
		sbrk(exceed);
	}

	K_HEAP_INDEX += n;
	return (uint32_t) heap_sz;
}

void k_heap_init() {
	uint32_t* phys = k_page_alloc();

	k_paging_map(phys, K_HEAP_BOTTOM, 0x3);

	K_HEAP_TOP = (uint32_t) K_HEAP_BOTTOM;
	sbrk(0x1000);
}