/* pmm_test.c */

#include <stdint.h>
#include <stdio.h>

#define TEST(num, bit)		(num & (1<<bit))
#define SET(num, bit)	(num |= (1<<bit))
#define CLEAR(num, bit)		(num &= ~(1<<bit))

/* 
One physical page is 4096 bytes (4 KB)
Each bit represents one page 
Each bitmap[n] represents 32 pages (128 KB)
Each struct block represents 1024 pages (4 MB)
*/

struct block {
	uint32_t bitmap[32];
} __attribute__((packed));
// 128 bytes


/* This would take up 32 physical pages... which is only 128 KB I guess*/
struct pages {
	struct block blocks[0x400] __attribute__((aligned(0x1000)));
} pp;

static struct mm_info {
	struct pages* p;
	uint32_t last_alloc;
	uint32_t last_free;
	uint32_t used_pages;
};

/* Find the first free page and return it's address*/
uint32_t find_and_allocate(struct pages* p) {
	for (int block_index = 0; block_index < 1024; block_index++) {
		uint32_t* bitmap = p->blocks[block_index].bitmap;
		for (int q = 0; q < 32; q++)
			if (bitmap[q] != ~0){	/* bitmap with a free bit */
				uint32_t free_bits = bitmap[q] ^ ~0x0;	/* Free bits */
				/* Test each bit */
				for (int j = 0; j < 32; j++)
					if (TEST(free_bits, j)) {
						SET(bitmap, j);
						int page = j + (q * 32) + (block_index * 1024);
						return page * 0x1000;
					}
			}
	}
	printf("NO FREE MEMORY!\n");
	return -1;
}

int find_and_mark(struct pages* p, uint32_t address, int val) {
	int i = (address / (0x00400000));				/* Each block represents 4 mb, which block index into page structure */
	int q = (address / 0x20000) % 32;				/* What is the bitmap array index? */
	int j = (address - (q*0x20000)) / 0x1000 % 32;	/* what bitmap bit are we? */

	uint32_t* bitmap = p->blocks[i].bitmap;

	if (val == 1) 
		SET(bitmap[q], j);
	if (val == 0)
		CLEAR(bitmap[q], j);

	return TEST(bitmap[q], j);
}

/* Allocates the next available physical address */
uint32_t mm_alloc() {
	return find_and_allocate(&pp);
}

/* De-allocates a specific physical address */
uint32_t mm_free(uint32_t address) {
	return find_and_mark(&pp, address, 0);
}

/* Request allocation of a specific physical address
Returns 1 on success, -1 on failure */
int mm_request(uint32_t address) {
	int used = find_and_mark(&pp, address, -1);
	if (!used)	
		return find_and_mark(&pp, address, 1);
	return -1;
}

int main(int argc, char* argv[]) {

	printf("sizeof pages: %d\n", sizeof(pp));

	for (int q = 0; q< 1024; q++)
		for(int i = 0; i < 32; i++)
			pp.blocks[q].bitmap[i] = ~0;

	printf("%+d\n", mm_request(0));
	mm_free(0);
	printf("%+d\n", mm_request(0));




}