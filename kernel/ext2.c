/*
ext2.c
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
*/
#include <ide.h>
#include <ext2.h>
#include <types.h>
#include <assert.h>

/* 	Read superblock from device dev, and check the magic flag.
	Return NULL if not a valid EXT2 partition */
superblock* ext2_superblock(int dev) {

	static superblock* sb = NULL;
	assert(dev);
	if(!dev)
		return NULL;
	if (sb)
		return sb;

	buffer* b = buffer_read(dev, EXT2_SUPER);
	sb = (superblock*) b->data;
	
	assert(sb->magic == EXT2_MAGIC);
	if (sb->magic != EXT2_MAGIC)
		return NULL;
	assert(1024 << sb->log_block_size == BLOCK_SIZE);
	return sb;
}

/* RW first superblock 
set s to NULL to read a superblock */
superblock* ext2_superblock_rw(int dev,  superblock* s) {
	assert(dev);
	if (!dev) 
		return NULL;
	buffer* b = buffer_read(dev, EXT2_SUPER);

	if (s->magic != EXT2_MAGIC) {	// Non-valid superblock, read 
		s = (superblock*) b->data;
	} else {							// Valid superblock, overwrite
		memcpy(b->data, s, sizeof(superblock));
		buffer_write(b);
	}
	
	assert(s->magic == EXT2_MAGIC);
	if (s->magic != EXT2_MAGIC)
		return NULL;
	assert(1024 << s->log_block_size == BLOCK_SIZE);
	return s;
}



block_group_descriptor* ext2_blockdesc(int dev) {
	assert(dev);
	if (!dev) 
		return NULL;

	buffer* b = buffer_read(dev, EXT2_SUPER + 1);
	block_group_descriptor* bg = (block_group_descriptor*) b->data;
	return bg;
}
/* Set bg to NULL to read the block group descriptor
Otherwise, overwrite it */
block_group_descriptor* ext2_blockdesc_rw(int dev, block_group_descriptor* bg) {
	assert(dev);
	if (!dev) 
		return NULL;
	buffer* b = buffer_read(dev, EXT2_SUPER + 1);
	if (bg == NULL) {
		bg = (block_group_descriptor*) b->data;
	} else {
		memcpy(b->data, bg, sizeof(block_group_descriptor));
		buffer_write(b);
	}
	return bg;
}

inode* ext2_inode(int dev, int i) {
	superblock* s = ext2_superblock(dev);
	block_group_descriptor* bgd = ext2_blockdesc(dev);

	assert(s->magic == EXT2_MAGIC);
	assert(bgd);

	int block_group = (i - 1) / s->inodes_per_group; // block group #
	int index 		= (i - 1) % s->inodes_per_group; // index into block group
	int block 		= (index * INODE_SIZE) / BLOCK_SIZE; 
	bgd += block_group;

	// Not using the inode table was the issue...
	buffer* b = buffer_read(dev, bgd->inode_table+block);
	inode* in = (inode*)((uint32_t) b->data + (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE);
	
	return in;
}

void* ext2_open(inode* in) {
	assert(in);
	if(!in)
		return NULL;
	int num_blocks = in->blocks / (BLOCK_SIZE/SECTOR_SIZE);	
	assert(num_blocks);
	if (!num_blocks){

		return NULL;
	}
	char* buf = malloc(BLOCK_SIZE*num_blocks);

	for (int i = 0; i < num_blocks; i++) {

		buffer* b = buffer_read(1, in->block[i]);
		memcpy((uint32_t)buf+(i*BLOCK_SIZE), b->data, BLOCK_SIZE);
	}

	return buf;
}

void ls(dirent* d) {
	do{
	//	d->name[d->name_len] = '\0';
		printf("\t%s\ttype %d\n", d->name, d->file_type);

		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
}

void lsroot() {
	inode* i = ext2_inode(1, 2);			// Root directory

	char* buf = malloc(BLOCK_SIZE*i->blocks/2);
	memset(buf, 0, BLOCK_SIZE*i->blocks/2);

	for (int q = 0; q < i->blocks / 2; q++) {
		buffer* b = buffer_read(1, i->block[q]);
		memcpy((uint32_t)buf+(q * BLOCK_SIZE), b->data, BLOCK_SIZE);
	}

	dirent* d = (dirent*) buf;
	
	int sum = 0;
	int calc = 0;
	do {
		
		// Calculate the 4byte aligned size of each entry
		calc = (sizeof(dirent) + d->name_len + 4) & ~0x3;
		sum += d->rec_len;

		if (d->rec_len != calc && sum == 1024) {
			/* if the calculated value doesn't match the given value,
			then we've reached the final entry on the block */
			sum -= d->rec_len;
	
			d->rec_len = calc; 		// Resize this entry to it's real size
			d = (dirent*)((uint32_t) d + d->rec_len);
			break;

		}
		printf("%d\t/%s\n", d->inode, d->name);

		d = (dirent*)((uint32_t) d + d->rec_len);


	} while(sum < 1024);

	free(buf);
	return NULL;
}

int ext_first_free(uint32_t* b, int sz) {
	for (int q = 0; q < sz; q++) {
		uint32_t free_bits = (b[q] ^ ~0x0);
		for (int i = 0; i < 32; i++ ) 
			if (free_bits & (0x1 << i)) 
				return i + (q*32);
	}
}

