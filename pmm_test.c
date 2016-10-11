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



int find_and_allocate(struct pages* p) {
	for (int i = 0; i < 1024; i++) {
		/* Do 4 byte comparisons */
		uint32_t* bitmap = p->blocks[i].bitmap;
		for (int q = 0; q < 32; q++)
			
			if (bitmap[q] != ~0){	/* bitmap with a free bit */
				uint32_t t = bitmap[q] ^ ~0x0;	/* Free bits */

				/* Test each bit */
				for (int j = 0; j < 32; j++)
					if (TEST(t, j)) {
						int f = j + (q * 32) + (i * 1024);
						SET(p->blocks[i].bitmap[q], j);
						printf("bit free %5d address %8x : free bits: %8x used bits: %8x\n", f, (f*0x1000), t, t ^ 0xFFFFFFFF);
						return f * 0x1000;
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

	if (val) 
		SET(bitmap[q], j);
	else
		CLEAR(bitmap[q], j);
}

int find_and_free(struct pages* p, uint32_t address) {
	int i = (address / (0x00400000));				/* Each block represents 4 mb, which block index into page structure */
	int q = (address / 0x20000) % 32;				/* What is the bitmap array index? */
	int j = (address - (q*0x20000)) / 0x1000 % 32;	/* what bitmap bit are we? */

	uint32_t* bitmap = p->blocks[i].bitmap;
	CLEAR(bitmap[q], j);
}




int main(int argc, char* argv[]) {

	printf("sizeof pages: %d\n", sizeof(pp));

	for (int q = 0; q< 1024; q++)
		for(int i = 0; i < 32; i++)
			pp.blocks[q].bitmap[i] = ~0;

	int x = 1023;
	pp.blocks[x].bitmap[2] = 0x0EFB;
	while(pp.blocks[x].bitmap[2] != ~0) {
		int f = find_and_allocate(&pp);	
		find_and_free(&pp, f);
	}



}