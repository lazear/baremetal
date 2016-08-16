/*
malloc.c

Michael Lazear, 2016

Experimental psuedo-bitmap dynamic memory management
32bit integer is used as the blockchain. Bit 32 is used (1) or free (0)
Bits 0:31 represent the size of the block.
*b = (1<<31); should set it to used
*b |= (size & ~(1<<31)); sets the size
b++; increments to the next block in the chain. 
Some simple math allows us to translate blockchain position to pointer address and vice-versa

The idea here is to alloc a static region of physical memory to store dynamic heap memory
The current memory layout is as follows:
0x00000000 - Bios data, we don't go here
0x00100000 - Start of kernel data and MM permanent heap (1MB)
0x00200000 - End of MM permanent heap (2MB)
Everything up to 4MB has been linearly mapped by paging for now,
so no need to map anything if we call mm_alloc() for block data - k_page_alloc maybe?

If we store the initial location of the mm_alloc call in a global variable and don't touch it,
we can know where the bottom of the block data starts, since we're not using linked lists

We can also store the top of the block data/how many blocks have been allocated total, 
this provides a maximum limit

We will not be storing the actual address of the memory allocated, we're gonna do some fancy 
tricks for that.

Since there is no buffer or randomness involved, this is a TRUST dependent malloc.
Do NOT use it for sensitive applications due to the high probability of overflow.

*/

#include <types.h>

const uint32_t MAX_BLOCKS				= 1024;

uint32_t BLOCKCHAIN_START	= 0;
uint32_t BLOCKS_ALLOCATED 	= 0;

uint32_t *blockchain		= NULL;

/*
Define limits of the heap. Bottom and Max never change. 
TOP should be 0x1000 aligned at all times
LAST_ALLOC is basically the current used top, the next address allocated will be LAST_ALLOC

|------ MAX -----|0xF0000000
|				 |
|				 |
|-------TOP------|0xC0001000
|-- LAST_ALLOC---|0xC0000CF3 etc etc
|				 |
|_____BOTTOM_____|0xC0000000
*/

const uint32_t K_HEAP_BOTTOM = 0x01000000;
const uint32_t K_HEAP_MAX = 0xF0000000;

uint32_t K_HEAP_TOP = 0;
uint32_t K_LAST_ALLOC =  0;


int blockchain_add(uint32_t size) {
	if (BLOCKS_ALLOCATED > MAX_BLOCKS)
		return -1;

	blockchain = BLOCKCHAIN_START + (BLOCKS_ALLOCATED * sizeof(uint32_t));
	*blockchain = (1 << 31) | (size & ~(1<<31));

	BLOCKS_ALLOCATED++;
	return 1;
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

/* 
Finds the first free block with size N bytes.
If there is no free block, return NULL
*/
void* find_first_free(size_t n) {
	uint32_t* block = (uint32_t*)BLOCKCHAIN_START;		// Reset the heap to the bottom
	uint32_t total_size = 0;

	for (int i = 0; i < BLOCKS_ALLOCATED; i++) {
		uint32_t block_value = *block;
		int used = (block_value >> 31);
		int size = (block_value & ~(1<<31));
		total_size += size;
		if (size >= n && !used) {
			printf("found free block (%d) with size %d matching request for %d\n", i, size, n);
			return block;
		}
		block++;
	}
	return NULL;
}

/* Find the smallest block that can fit N bytes. Max size is N*10 */
void* find_best_free(size_t n) {
	uint32_t* block = (uint32_t*)BLOCKCHAIN_START;		// Reset the heap to the bottom
	uint32_t total_size = 0;

	uint32_t* smallest = NULL;
	uint32_t smallest_sz = n*3;
loop:
	for (int i = 0; i < BLOCKS_ALLOCATED; i++) {
		uint32_t block_value = *block;
		int used = (block_value >> 31);
		int size = (block_value & ~(1<<31));
		total_size += size;
		if (size >= n && size <= smallest_sz && !used) {
		//	printf("found free block (%d) with size %d matching request for %d\n", i, size, n);
			smallest = block++;
			smallest_sz = size;
			goto loop;
		}
		block++;
	}
	return smallest;
}

extern void* translate(uint32_t* block);

void blockinfo(uint32_t* block) {
	uint32_t block_value = *block;
	int used = (block_value >> 31);
	int size = (block_value & ~(1<<31));
	int i = ((uint32_t) block - BLOCKCHAIN_START) / sizeof(uint32_t);
	printf("Block (%d) 0x%x | Used: %d | Size: %d | Link 0x%x\n", i, block, used, size, translate(block));
}

uint32_t heap_brk() {
	return K_HEAP_TOP;
}

void traverse_blockchain() {
	uint32_t* block = (uint32_t*)BLOCKCHAIN_START;		// Reset the heap to the bottom
	uint32_t total_size = 0;

	for (int i = 0; i < BLOCKS_ALLOCATED; i++) {
		uint32_t block_value = *block;
		int used = (block_value >> 31);
		int size = (block_value & ~(1<<31));
		total_size += size;
		printf("Block (%d) 0x%x | Used: %d | Size: %d | Link 0x%x\n", i, block, used, size, translate(block));
		block++;
	}
	printf("%d bytes allocated across %d blocks\n", (K_LAST_ALLOC - K_HEAP_BOTTOM), BLOCKS_ALLOCATED);
	printf("%x HEAP_BRK, %x LAST_ALLOC\n", K_HEAP_TOP, K_LAST_ALLOC);
}

/* Determines the location of an address in the blockchain */
uint32_t* find_block(void* ptr) {
	if ((uint32_t)ptr > K_LAST_ALLOC)
		return NULL;

	uint32_t to_traverse = (uint32_t) ptr - K_HEAP_BOTTOM;
	uint32_t* block = (uint32_t*)BLOCKCHAIN_START;		// Reset the heap to the bottom
	uint32_t total_size = 0;
	for (int i = 0; i < BLOCKS_ALLOCATED; i++) {
		uint32_t block_value = *block++;
		int used = (block_value >> 31);
		int size = (block_value & ~(1<<31));
		total_size += size;
		if (total_size > to_traverse)
			break;		// Go one block over what we wanted, so decrease to the last heap.
	}

	block--;
	return block;
}

/* Translate blockchain address to virtual address */
void* translate(uint32_t* b) {
	if ((uint32_t) b > (BLOCKS_ALLOCATED * sizeof(uint32_t) + BLOCKCHAIN_START))
		return NULL;

	uint32_t* block = (uint32_t*)BLOCKCHAIN_START;		// Reset the heap to the bottom
	uint32_t total_size = 0;

	while(block != b) {
		uint32_t block_value = *block++;
		int used = (block_value >> 31);
		int size = (block_value & ~(1<<31));
		total_size += size;
	}

	uint32_t* address = K_HEAP_BOTTOM + total_size;
	if ((uint32_t) address > K_LAST_ALLOC) {
		printf("Translation error %s:%d", __FILE__, __LINE__);
		return NULL;
	}
	return address;
}

/*
free() finds the block referenced by the pointer and sets bit 31 to 0.
*/
void* free(void* ptr) {
	uint32_t* block = find_block(ptr);
	*block &= ~(1<<31);
	return ptr;
}

/*
malloc() will try to find an empty block of best fit.
If there are no appropriate free blocks, a new one will be allocated
*/
void* malloc(size_t n) {
	void* ptr = NULL;
	uint32_t* block = NULL;

	if (!n)
		return ptr;
/*	if (n < ARBITRARY_SEARCH_CUTOFF)
		block = find_best_free(n);
	else
		block = find_first_free(n);*/
	block = find_best_free(n);

	if (block) {
		/* We found a free block */
		*block |= (1<<31);
		ptr = translate(block);
		//printf("Reuse, reduce, recycle %d bytes: @ 0x%x\n", n, ptr);
	} else {
		/* We couldn't find a free block, allocate a new one */
		
		int r = blockchain_add(n);
		if (r < 0)
			return NULL;

		// Check if we need to call sbrk
		if (K_LAST_ALLOC + n >= K_HEAP_TOP) 
			sbrk((n + K_LAST_ALLOC) - K_HEAP_TOP);

		ptr = (void*) K_LAST_ALLOC;
		K_LAST_ALLOC += n;
	}
	//printf("Allocated %d bytes\n", n);
	//blockinfo(translate(ptr));
	return ptr;
}


/* 
Since this memory model doesn't permit resizing:
1. allocate a new pointer of size N
2. copy data from that ptr to the new one (bytes = smallest size)
3. free the old pointer
*/
void* realloc(void* ptr, size_t n) {

	uint32_t* new = malloc(n);
	uint32_t* b = find_block(ptr);
	if (!b) {
		printf("Not in malloc? %x\n", ptr);
		return NULL;
	}

	uint32_t bv = *b;
	*b = (bv & ~(1<<31));
	uint32_t sz = ( bv & ~(1<<31));
	if (n <= sz)
		memcpy(new, ptr, n);
	else
		memcpy(new, ptr, sz);

	printf("Reallocating 0x%x (%d) to 0x%x (%d)\n", (uint32_t)ptr, sz, new, n);
	return new;
	//(block_value 
}


void k_heap_init() {

	blockchain = mm_alloc(MAX_BLOCKS * sizeof(uint32_t));	// this should give us 1024 blocks in the 1MB-2MB range
	memset(blockchain, 0, MAX_BLOCKS * sizeof(uint32_t));	// Un-freeable memory


	BLOCKCHAIN_START = (uint32_t) blockchain;
	K_HEAP_TOP = K_HEAP_BOTTOM;
	K_LAST_ALLOC = K_HEAP_BOTTOM;

	sbrk(0x4000);
}

void k_heap_test() {

	/*
	Malloc() exact string length and then reallocing immediately
	causes some buffer overflow issues.
	Perhaps should add in an extra 4 bytes of buffer between malloc()s
	automatically? Should help prevent some accidentall overflows.
	*/
	char d[] = "Welcome to baremetal - Michael Lazear";
	char* name = malloc(strlen(d)-1);
	char* r = malloc(4);
	printf("%s, %d chars\n", d, strlen(d));
	strcpy(name, d);
	char* two = realloc(name, 50);
	printf("%s] -> [%s\n", name, two);
	printf("%s\n", r);
//	printf("New: %s\n", new);
	traverse_blockchain();

}