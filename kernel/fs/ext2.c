/*
ext2util.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================

ext2util is a command-line interface for reading/writing data from ext2
disk images. ext2 driver code is directly ported from my own code used in a
hobby operating system. buffer_read and buffer_write are "glue" functions
allowing the ramdisk to emulate a hard disk
*/

#include <ext2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define NULL ((void*) 0)
static int fp = NULL;


/* Overloads for superblock read/write, since it is ALWAYS 1024 bytes in
from the beginning of the disk, regardless of logical block size */
buffer* buffer_read_superblock(struct ext2_fs* f) {
	int block = (f->block_size == 1024) ? 1 : 0;
	return buffer_read(f, block);
}

uint32_t buffer_write_superblock(struct ext2_fs *f, buffer* b) {
	buffer_write(f, b);
}


/* 	Read superblock from device dev, and check the magic flag.
	Updates the filesystem pointer */
int ext2_superblock_read(struct ext2_fs *f) {
	if (!f)
		return -1;
	if (!f->sb)
		f->sb = malloc(sizeof(superblock));

	buffer* b = buffer_read_superblock(f);
	memcpy(f->sb, b->data, sizeof(superblock));
	buffer_free(b);

	#ifdef DEBUG
	printf("Reading superblock\n");
	#endif
	assert(f->sb->magic == EXT2_MAGIC);
	if (f->sb->magic != EXT2_MAGIC) {
		printf("ABORT: INVALID SUPERBLOCK\n");
		return NULL;
	}
	f->block_size = (1024 << f->sb->log_block_size);
	return 0;
}

/* Sync superblock to disk */
int ext2_superblock_write(struct ext2_fs *f) {

	if (!f)
		return -1;
	if (f->sb->magic != EXT2_MAGIC) {	// Non-valid superblock, read 
		return -1;
	} else {						// Valid superblock, overwrite
		#ifdef DEBUG
		printf("Writing to superblock\n");
		#endif
		buffer* b = buffer_read_superblock(f);
		memcpy(b->data, f->sb, sizeof(superblock));
		buffer_write_superblock(f, b);
		buffer_free(b);
	}
	return 0;
}

int ext2_blockdesc_read(struct ext2_fs *f) {
	if (!f) return -1;

	int num_block_groups = (f->sb->blocks_count / f->sb->blocks_per_group);
	int num_to_read = (num_block_groups * sizeof(block_group_descriptor)) / f->block_size;
	f->num_bg = num_block_groups;
	num_to_read++;	// round up

	#ifdef DEBUG
	printf("Number of block groups: %d (%d blocks)\n", f->num_bg, num_to_read);
	#endif

	if (!f->bg) {
		f->bg = malloc(num_block_groups* sizeof(block_group_descriptor));
	}

	/* Above a certain block size to disk size ratio, we need more than one block */
	for (int i = 0; i < num_to_read; i++) {
		int n = EXT2_SUPER + i + ((f->block_size == 1024) ? 1 : 0); 	
		buffer* b = buffer_read(f, n);
		memcpy((uint32_t) f->bg + (i*f->block_size), b->data, f->block_size);
		buffer_free(b);
	}
	return 0;
}

int ext2_blockdesc_write(struct ext2_fs *f) {
	if (!f) return -1;

	int num_block_groups = (f->sb->blocks_count / f->sb->blocks_per_group);
	int num_to_read = (num_block_groups * sizeof(block_group_descriptor)) / f->block_size;
	/* Above a certain block size to disk size ratio, we need more than one block */
	num_to_read++;	// round up
	for (int i = 0; i < num_to_read; i++) {
		int n = EXT2_SUPER + i + ((f->block_size == 1024) ? 1 : 0); 	
		buffer* b = buffer_read(f, n);
		memcpy(b->data, (uint32_t) f->bg + (i*f->block_size), f->block_size);

		#ifdef DEBUG
		printf("Writing to block group desc (block %d)\n", n);
		#endif
		buffer_write(f, b);
		buffer_free(b);
	}
	return 0;
}


int ext2_first_free(uint32_t* b, int sz) {
	for (int q = 0; q < sz; q++) {
		uint32_t free_bits = (b[q] ^ ~0x0);
		for (int i = 0; i < 32; i++ ) 
			if (free_bits & (0x1 << i)) 
				return i + (q*32);
	}
	return -1;
}

/* 
Finds a free block from the block descriptor group, and sets it as used
*/
uint32_t ext2_alloc_block(struct ext2_fs *f, int block_group) {
	block_group_descriptor* bg = f->bg;
	superblock* s = f->sb;

	bg += block_group;
	// Read the block and inode bitmaps from the block descriptor group
	buffer* bitmap_buf;
	uint32_t* bitmap = malloc(f->block_size);
	uint32_t num = 0;
	do {

		bitmap_buf = buffer_read(f, bg->block_bitmap);
		memcpy(bitmap, bitmap_buf->data, f->block_size);
		// Find the first free bit in both bitmaps
		bg++;
		block_group++;
	} while( (num = ext2_first_free(bitmap, f->block_size)) == -1);	

	// Should use a macro, not "32"
	bitmap[num / 32] |= (1<<(num % 32));

	// Update bitmaps and write to disk
	memcpy(bitmap_buf->data, bitmap, f->block_size);	
	buffer_write(f, bitmap_buf);		

	// Free our bitmaps
	free(bitmap);			

	s->free_inodes_count--;
	bg->free_inodes_count--;
	buffer_free(bitmap_buf);
	return num + ((block_group - 1) * s->blocks_per_group) + 1;	// 1 indexed				
}

void ext2_write_indirect(struct ext2_fs *f, uint32_t indirect, uint32_t link, size_t block_num) {
	buffer* b = buffer_read(f, indirect);
	*(uint32_t*) ((uint32_t) b->data + block_num*4)  = link;
	buffer_write(f, b);
}

uint32_t ext2_read_indirect(struct ext2_fs *f, uint32_t indirect, size_t block_num) {
	buffer* b = buffer_read(f, indirect);
	return *(uint32_t*) ((uint32_t) b->data + block_num*4);
}
