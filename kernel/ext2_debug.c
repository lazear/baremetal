/*
ext2_debug.c
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


block_group_descriptor* bg_dump(block_group_descriptor* bg) {
	printf("Block bitmap %d\n", bg->block_bitmap);
	printf("Inode bitmap %d\n", bg->inode_bitmap);
	printf("Inode table  %d\n", bg->inode_table);
	printf("Free blocks  %d\n", bg->free_blocks_count);
	printf("Free inodes  %d\n", bg->free_inodes_count);
	printf("Used dirs    %d\n", bg->used_dirs_count);
	
	buffer* bbm = buffer_read(1, bg->block_bitmap);
	buffer* ibm = buffer_read(1, bg->inode_bitmap);

	printf("First free BBM: %d\n", ext_first_free(bbm->data, 1024));
	printf("First free IBM: %d\n", ext_first_free(ibm->data, 1024));
	return bg;
}


/* 
Finds a free block from the block descriptor group, and sets it as used
*/
uint32_t ext2_alloc_block() {
	block_group_descriptor* bg = ext2_blockdesc(1);
	// Read the block and inode bitmaps from the block descriptor group
	buffer* bitmap_buf = buffer_read(1, bg->block_bitmap);
	uint32_t* bitmap = malloc(BLOCK_SIZE);
	memcpy(bitmap, bitmap_buf->data, BLOCK_SIZE);

	// Find the first free bit in both bitmaps
	uint32_t num = ext_first_free(bitmap, BLOCK_SIZE);

	// Should use a macro, not "32"
	bitmap[num / 32] |= (1<<(num % 32));

	// Update bitmaps and write to disk
	memcpy(bitmap_buf->data, bitmap, BLOCK_SIZE);	
	buffer_write(bitmap_buf);								
	// Free our bitmaps
	free(bitmap);				
	return num + 1;	// 1 indexed				
}

/* 
Finds a free inode from the block descriptor group, and sets it as used
*/
uint32_t ext2_alloc_inode() {
	block_group_descriptor* bg = ext2_blockdesc(1);
	// Read the block and inode bitmaps from the block descriptor group
	buffer* bitmap_buf = buffer_read(1, bg->inode_bitmap);
	uint32_t* bitmap = malloc(BLOCK_SIZE);
	memcpy(bitmap, bitmap_buf->data, BLOCK_SIZE);

	// Find the first free bit in both bitmaps
	uint32_t num = ext_first_free(bitmap, BLOCK_SIZE);

	// Should use a macro, not "32"
	bitmap[num / 32] |= (1<<(num % 32));

	// Update bitmaps and write to disk
	memcpy(bitmap_buf->data, bitmap, BLOCK_SIZE);	
	buffer_write(bitmap_buf);								
	// Free our bitmaps
	free(bitmap);				
	return num + 1;	// 1 indexed				
}

void ext2_write_indirect(uint32_t indirect, uint32_t link, size_t block_num) {
	buffer* b = buffer_read(1, indirect);
	*(uint32_t*) ((uint32_t) b->data + block_num*4)  = link;
	buffer_write(b);
}

uint32_t ext2_read_indirect(uint32_t indirect, size_t block_num) {
	buffer* b = buffer_read(1, indirect);
	return *(uint32_t*) ((uint32_t) b->data + block_num*4);
}


int ext2_touch(char* name, char* data, size_t n) {
	/* 
	Things we need to do:
		* Find the first free inode # and free blocks needed
		* Mark that inode and block as used
		* Allocate a new inode and fill out the struct
		* Write the new inode to the inode_table
		* Update superblock&blockgroup w/ last mod time, # free inodes, etc
		* Update directory structures
	*/

	superblock* s = ext2_superblock(1);
	block_group_descriptor* bg = ext2_blockdesc(1);
	uint32_t inode_num = ext2_alloc_inode();

	int sz = n;
	int indirect = 0;

	/* Allocate a new inode and set it up
	*/
	inode* i = malloc(sizeof(inode));
	i->mode = 0x81C0;		// File, User mask
	i->size = n;
	i->atime = time();
	i->ctime = time();
	i->mtime = time();
	i->dtime = 0;
	i->links_count = 1;		/* Setting this to 0 = BIG NO NO */
/* SECTION: BLOCK ALLOCATION AND DATA WRITING */
	int q = 0;

	while(sz) {
		uint32_t block_num = ext2_alloc_block();

		// Do we write BLOCK_SIZE or sz bytes?
		int c = (sz >= BLOCK_SIZE) ? BLOCK_SIZE : sz;

		if (q < 12) {
			i->block[q] = block_num;
		} else if (q == 12) {
			indirect = block_num;
			i->block[q] = indirect;
			printf("Indirect block allocated: %d\n", indirect);
			block_num = ext2_alloc_block();
			printf("Next data block allocated: %d\n", block_num);
			ext2_write_indirect(indirect, block_num, 0);
		} else if(q > 12 && q < ((BLOCK_SIZE/sizeof(uint32_t)) + 12)) {
			ext2_write_indirect(indirect, block_num, q - 12);
		}

		i->blocks += 2;			// 2 sectors per block


		/* Go ahead and write the data to disk */
		buffer* b = buffer_read(1, block_num);
		memset(b->data, 0, BLOCK_SIZE);
		memcpy(b->data, (uint32_t) data + (q * BLOCK_SIZE), c);
		buffer_write(b);
		printf("[%d]\twrite from offset %d to block %d, indirect %d\n", q, q*BLOCK_SIZE, block_num, indirect);
		q++;
		sz -= c;	// decrease bytes to write
		s->free_blocks_count--;		// Update Superblock
		bg->free_blocks_count--;	// Update BG

	}
	ext2_write_indirect(indirect, 0, i->blocks/2);
	i->blocks+=2;
	printf("%d\n", i->blocks);
	for (int q = 0; q <= i->blocks/2; q++) {
		if (q < 12)
			printf("Direct   Block[%d] = %d\n", q, i->block[q]);
		else if (q == 12)
			printf("Indirect Block is @= %d\n", i->block[q]);
		else if (q > 12)
			printf("Indirect Block[%d] = %d\n", q - 13, ext2_read_indirect(indirect, q-13));
	}


/* SECTION: INODE ALLOCATION AND DATA WRITING */
	int block_group = (inode_num - 1) / s->inodes_per_group; // block group #
	int index 		= (inode_num - 1) % s->inodes_per_group; // index into block group
	int block 		= (index * INODE_SIZE) / BLOCK_SIZE; 
	int offset 		= (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE;
	bg += block_group;

	//printf("Inode %d:\tGroup %d\tIndex %d\tOffset into table %d\n", inode_num, block_group, index, bg->inode_table+block);
	//printf("Offset %x\n", offset);

	// Not using the inode table was the issue...
	buffer* ib = buffer_read(1, bg->inode_table+block);

	memcpy((uint32_t) ib->data + offset, i, INODE_SIZE);
	buffer_write(ib);

/* SECTION: UPDATE DIRECTORY */
	inode* rootdir = ext2_inode(1, 2);
	buffer* rootdir_buf = buffer_read(1, rootdir->block[0]);

	dirent* d = (dirent*) rootdir_buf->data;
	int sum = 0;
	int calc = 0;
	int new_entry_len = (sizeof(dirent) + strlen(name) + 4) & ~0x3;
	do {
		
		// Calculate the 4byte aligned size of each entry
		calc = (sizeof(dirent) + d->name_len + 4) & ~0x3;
		sum += d->rec_len;

		if (d->rec_len != calc && sum == 1024) {
			/* if the calculated value doesn't match the given value,
			then we've reached the final entry on the block */
			sum -= d->rec_len;
			if (sum + new_entry_len > 1024) {
				/* we need to allocate another block for the parent
				directory*/
				printf("PANIC! out of room");
				return NULL;
			}
			d->rec_len = calc; 		// Resize this entry to it's real size
			d = (dirent*)((uint32_t) d + d->rec_len);
			break;

		}
		//printf("name %s\n",d->name);
		d = (dirent*)((uint32_t) d + d->rec_len);


	} while(sum < 1024);

	/* d is now pointing at a blank entry, right after the resized last entry */
	d->rec_len = BLOCK_SIZE - ((uint32_t)d - (uint32_t)rootdir_buf->data);
	d->inode = inode_num;
	d->file_type = 1;
	d->name_len = strlen(name);

	/* memcpy() causes a page fault */
	for (int q = 0; q < strlen(name); q++) 
		d->name[q] = name[q];

	/* Write the buffer to the disk */
	buffer_write(rootdir_buf);

	/* Update superblock information */
	s->free_inodes_count--;
	s->wtime = time(NULL);
	ext2_superblock_rw(1,s);

	/* Update block group descriptors */
	bg->free_inodes_count--;
	ext2_blockdesc_rw(1, bg);

	//free(i);
	return inode_num;
}


void inode_dump(inode* in) {

	printf("Mode\t%x\t", in->mode);			// Format of the file, and access rights
	printf("UID\t%x\t", in->uid);			// User id associated with file
	printf("Size\t%d\n", in->size);			// Size of file in bytes
	printf("Atime\t%x\t", in->atime);			// Last access time, POSIX
	printf("Ctime\t%x\t", in->ctime);			// Creation time
	printf("Mtime\t%x\n", in->mtime);			// Last modified time
	printf("Dtime\t%x\t", in->dtime);			// Deletion time
	printf("Group\t%x\t", in->gid);			// POSIX group access
	printf("#Links\t%d\n", in->links_count);	// How many links
	printf("Sector\t%d\t", in->blocks);		// # of 512-bytes blocks reserved to contain the data
	printf("Flags\t%x\n", in->flags);			// EXT2 behavior
//	printf("%x\n", in->osdl);			// OS dependent value
	printf("Block1\t%d\n", in->block[0]);		// Block pointers. Last 3 are indirect

//	printf("%x\n", i->generation);	// File version
//	printf("%x\n", i->file_acl);		// Block # containing extended attributes
//	printf("%x\n", i->dir_acl);
//	printf("%x\n", i->faddr);			// Location of file fragment
//	printf("%x\n", i->osd2[3]);
}


// For debugging purposes
void sb_dump(struct superblock_s* sb) {
	printf("EXT2 Magic  \t%x\n", sb->magic);
	printf("Inodes Count\t%d\t", sb->inodes_count);
	printf("Blocks Count\t%d\t", sb->blocks_count);
	printf("RBlock Count\t%d\n", sb->r_blocks_count);
	printf("FBlock Count\t%d\t", sb->free_blocks_count); 	;
	printf("FInode Count\t%d\t", sb->free_inodes_count);
	printf("First Data B\t%d\n", sb->first_data_block);
	printf("Block Sz    \t%d\t", 1024 << sb->log_block_size);
	printf("Fragment Sz \t%d\t", 1024 << sb->log_frag_size);
	printf("Block/Group \t%d\n", sb->blocks_per_group);
	printf("Frag/Group  \t%d\t", sb->frags_per_group);
	printf("Inode/Group \t%d\t", sb->inodes_per_group);
	printf("mtime;\t%x\n", sb->mtime);
	printf("wtime;\t%x\t", sb->wtime);
	printf("Mnt #\t%x\t", sb->mnt_count);
	printf("Max mnt\t%x\n", sb->max_mnt_count);
	printf("State\t%x\t", sb->state);
	printf("Errors\t%x\t", sb->errors);
	printf("Minor\t%x\n", sb->minor_rev_level);
	printf("Last Chk\t%x\t", sb->lastcheck);
	printf("Chk int\t%x\t", sb->checkinterval);
	printf("OS\t%x\n", sb->creator_os);
	printf("Rev #\t%x\t", sb->rev_level);
	printf("ResUID\t%x\t", sb->def_resuid);
	printf("ResGID\t%x\n", sb->def_resgid);
}
