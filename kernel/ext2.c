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

typedef struct device_info_s {
	int id;
	superblock* sb;
	block_group_descriptor* bg;
} device;

device *rootdev = NULL;


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

block_group_descriptor* ext2_blockdesc(int dev) {
	assert(dev);
	if (!dev) 
		return NULL;

	buffer* b = buffer_read(dev, EXT2_SUPER + 1);
	block_group_descriptor* bg = (block_group_descriptor*) b->data;
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
//	printf("Inode %d:\tGroup %d\tIndex %d\tOffset into table %d\n", i, block_group, index, bgd->inode_table+block);
//	printf("Offset %x\n", (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE );

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


inode* ext2_lookup(char* name) {
	inode* i = ext2_inode(1, 2);			// Root directory
	buffer* b = buffer_read(1, i->block[0]);

	char* buf = malloc(BLOCK_SIZE);
	memcpy(buf, b->data, BLOCK_SIZE);

	dirent* d = (dirent*) buf;

	do{
	//	printf("name: %s\t", d->name);
		d->name[d->name_len] = '\0';
		if (strncmp(d->name, name, strlen(name)) == 0) {
			printf("Match! %s\n", d->name);
			printf("Inode %d\n", d->inode);
			return ext2_inode(1, d->inode);
		}
		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
	return NULL;
}

dirent* ext2_open_dir(int in) {
	inode* i = ext2_inode(1, in);
	char* buf = malloc(BLOCK_SIZE*i->blocks/2);
	for (int q = 0; q < i->blocks / 2; q++) {
		buffer* b = buffer_read(1, i->block[q]);
		memcpy((uint32_t)buf+(q * BLOCK_SIZE), b->data, BLOCK_SIZE);
	}
	return (dirent*) buf;
}

void ls(dirent* d) {
	do{
		d->name[d->name_len] = '\0';
		printf("\t%s\ttype %d\n", d->name, d->file_type);

		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
}

void lsroot() {
	inode* i = ext2_inode(1, 2);			// Root directory

	char* buf = malloc(BLOCK_SIZE*i->blocks/2);
	for (int q = 0; q < i->blocks / 2; q++) {
		buffer* b = buffer_read(1, i->block[q]);
		memcpy((uint32_t)buf+(q * BLOCK_SIZE), b->data, BLOCK_SIZE);
	//	printf("%d\t", i->block[q]);
	}

	dirent* d = (dirent*) buf;
	
	do{
		d->name[d->name_len] = '\0';
		printf("name: %s\tinode %d\n", d->name, d->inode);
		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
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

void ext2_init(int dev) {
	if(!IDE_STATUS) {
		panic("IDE device not found");
		return;
	}
	rootdev = malloc(sizeof(device));
	rootdev->id = dev;
	rootdev->sb = malloc(sizeof(superblock));
	rootdev->bg = malloc(sizeof(block_group_descriptor));
	memcpy(rootdev->sb, ext2_superblock(dev), sizeof(superblock));
	memcpy(rootdev->bg, ext2_blockdesc(dev), sizeof(block_group_descriptor));
}