/*
Heap functions
malloc()
free()
sbrk()
brk()
*/

#include <types.h>
#include <vga.h> 	// for nice color definitions

uint32_t PAGE_FRAME_BEGIN = 0;

struct  KHEAPBLOCK {
	struct KHEAPBLOCK	*next;
	uint32_t	size;
	uint8_t		used : 1
} __attribute__((__packed__)) block;


// QUICK and DIRTY alloc function

const uint32_t* K_HEAP_BOTTOM = 0xC0000000;
const uint32_t K_HEAP_MAX = 0xF0000000;

uint32_t K_HEAP_TOP = 0;

int K_HEAP_INDEX = 0;


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
	if (n + K_HEAP_TOP >= K_HEAP_MAX) {
		printf("HEAP FULL\n");
		return NULL;
	}

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


void* malloc(size_t n) {
	void* r = wf_malloc(n);
//	printf("Allocating %d bytes @ 0x%x\n", n, r);
	return r;
}

void k_heap_init() {
	uint32_t* phys = k_page_alloc();

	k_paging_map(phys, K_HEAP_BOTTOM, 0x3);

	K_HEAP_TOP = (uint32_t) K_HEAP_BOTTOM;
	sbrk(0x1000);
	printf("Block size: %d bytes", sizeof(block));
}

void heap_test() {
	/*
	allocate a bunch of memory, and then scroll through it, 
	trying to throw a page fault
	*/
	uint32_t* bot = k_heap_top();
	for (int i = 0; i < 8; i++) {
		uint32_t* t = k_page_alloc();
		k_page_alloc();
		k_page_alloc();
		uint32_t* x = wf_malloc(0x900);

		k_page_free(t);
		if (t != k_page_alloc())
			vga_pretty("Error with page_alloc() and page_free()", VGA_RED);
	}
	uint32_t* top = k_heap_top();

	for (int i = (uint32_t) bot; i < (uint32_t) top - 8; i++) {
		(*(uint32_t*) i) = 0;
	}
	vga_pretty("Heap test successful!\n", VGA_LIGHTGREEN);
}