/*
Heap functions
malloc()
free()
sbrk()
brk()
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
