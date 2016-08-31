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

	uint32_t* block_bitmap = malloc(BLOCK_SIZE);
	uint32_t* inode_bitmap = malloc(BLOCK_SIZE);

	memcpy(block_bitmap, bbm->data, BLOCK_SIZE);
	memcpy(inode_bitmap, ibm->data, BLOCK_SIZE);

	// for (int i = 0; i < BLOCK_SIZE/4; i += 4) 
	// 	block_bitmap[i] = byte_order(*(uint32_t*)((uint32_t)bbm->data + i));
	// for (int i = 0; i < BLOCK_SIZE/4; i += 4) 
	// 	inode_bitmap[i] = byte_order(*(uint32_t*)((uint32_t)ibm->data + i));


	printf("First free BBM: %d\n", ext_first_free(block_bitmap, 1024));
	printf("First free IBM: %d\n", ext_first_free(inode_bitmap, 1024));

	//for (int i =0; i < 20; i++) {
//	buffer* c = buffer_read(dev, 18);
	//	int ticks = get_ticks();
	//while(ticks + 20 > get_ticks());
	//buffer_dump(c);
	//}

	return bg;
}


void ext2_touch() {
	/* 
	Things we need to do:
		* Find the first free inode # and free blocks needed
		* Mark that inode and block as used
		* Allocate a new inode and fill out the struct
		* Write the new inode to the inode_table
		* Update superblock&blockgroup w/ last mod time, # free inodes, etc
		* Update directory structures
	*/

	block_group_descriptor* bg = ext2_blockdesc(1);
	superblock* s = ext2_superblock(1);

	buffer* bbm = buffer_read(1, bg->block_bitmap);
	buffer* ibm = buffer_read(1, bg->inode_bitmap);

	uint32_t* block_bitmap = malloc(BLOCK_SIZE);
	uint32_t* inode_bitmap = malloc(BLOCK_SIZE);

	memcpy(block_bitmap, bbm->data, BLOCK_SIZE);
	memcpy(inode_bitmap, ibm->data, BLOCK_SIZE);

	uint32_t block_num = ext_first_free(block_bitmap, 1024);
	uint32_t inode_num = ext_first_free(inode_bitmap, 1024);


	char* data = "My name is Michael and this is an inode!";

	block_bitmap[block_num / 32] |= (1<<(block_num % 32));
	inode_bitmap[inode_num / 32] |= (1<<(inode_num % 32));

	memcpy(bbm->data, block_bitmap, BLOCK_SIZE);	// Update bitmaps
	memcpy(ibm->data, inode_bitmap, BLOCK_SIZE);

	buffer_write(bbm);								// Write to disk
	buffer_write(ibm);
	free(block_bitmap);								// Free()
	free(inode_bitmap);

	/*
	Inodes are indexed starting at 1
	bitmap does not take this into account
	*/
	inode_num++;		
	block_num++;

	/* Allocate a new inode and set it up
	*/
	inode* i = malloc(sizeof(inode));
	i->mode = 0x81C0;		// File, User mask
	i->size = strlen(data);
	i->atime = time();
	i->ctime = time();
	i->mtime = time();
	i->links_count = 0;
	i->block[0] = block_num;
	i->blocks = 2;			// 2 sectors per block

	buffer* b = buffer_read(1, block_num);
	memset(b->data, 0, BLOCK_SIZE);
	memcpy(b->data, data, strlen(data));

	buffer_write(b);


	int block_group = (inode_num - 1) / s->inodes_per_group; // block group #
	int index 		= (inode_num - 1) % s->inodes_per_group; // index into block group
	int block 		= (index * INODE_SIZE) / BLOCK_SIZE; 
	bg += block_group;

	//printf("Inode %d:\tGroup %d\tIndex %d\tOffset into table %d\n", inode_num, block_group, index, bg->inode_table+block);
	//printf("Offset %x\n", (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE );

	// Not using the inode table was the issue...
	buffer* ib = buffer_read(1, bg->inode_table+block);
	memcpy((uint32_t) ib->data + (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE, i, INODE_SIZE);
	buffer_write(ib);

	inode* rootdir = ext2_inode(1, 2);

	buffer* rootdir_buf = buffer_read(1, rootdir->block[0]);
	printf("roodir block %d\n", rootdir->block[0]);
	dirent* d = (dirent*) rootdir_buf->data;
	int sum = 0;
	int calc = 0;
	do {
		
		//d->name[d->name_len] = '\0';
	if (!d->rec_len)
			break;

	
		calc = (sizeof(dirent) + d->name_len + 4) & ~0x3;
	//	printf("name: %s\tinode %d\trec %d\ttype %d\n", d->name, d->inode, d->rec_len, d->file_type);
		sum += d->rec_len;
		if (d->rec_len != calc) {
			sum -= d->rec_len;
			d->rec_len = calc;
			printf("LAST name: %s\tinode %d\trec %d\ttype %d\n", d->name, d->inode, d->rec_len, d->file_type);

			d = (dirent*)((uint32_t) d + d->rec_len);
printf("LAST name: %s\tinode %d\trec %d\ttype %d\n", d->name, d->inode, d->rec_len, d->file_type);
	
			break;
		}

		d = (dirent*)((uint32_t) d + d->rec_len);


	} while(sum < 1024);
	//dirent* entry = malloc((sizeof(dirent) + strlen("newfile") + 4) & ~0x3);
	d->rec_len = BLOCK_SIZE - ((uint32_t)d - (uint32_t)rootdir_buf->data);
	d->inode = inode_num;
	d->file_type = 1;
	d->name[0] = 'N';
//	memcpy(d->name, "newfile");
	d->name_len = strlen("1");


	buffer_write(rootdir_buf);

	printf("name: %s\tinode %d\trec %d\ttype %d\n", d->name, d->inode, d->rec_len, d->file_type);
		

	s->free_blocks_count--;
	s->free_inodes_count--;
	s->wtime = time();
	ext2_superblock_rw(1,s);
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
