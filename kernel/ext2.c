
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
	//printf("%d:\t%d\t%d\t%d\n", i, block_group, index, bgd->inode_table+block);
	// Not using the inode table was the issue...
	buffer* b = buffer_read(dev, bgd->inode_table + block);
	inode* in = (inode*)((uint32_t) b->data + (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE);
	return in;
}


inode* ext2_lookup(char* name) {
	inode* i = ext2_inode(1, 2);			// Root directory
	buffer* b = buffer_read(1, i->block[0]);

	char* buf = malloc(1024);
	memcpy(buf, b->data, 1024);

	dirent* d = (dirent*) buf;

	do{
	//	printf("name: %s\t", d->name);

		if (strcmp(d->name, name) == 0) {
			printf("Match! %s\n", d->name);
			return ext2_inode(1, d->inode);
		}
		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
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

block_group_descriptor* bgd(superblock*s, int dev) {

	block_group_descriptor* bg = ext2_blockdesc(dev);
	
/*	printf("Block bitmap %d\n", bg->block_bitmap);
	printf("Inode bitmap %d\n", bg->inode_bitmap);
	printf("Inode table  %d\n", bg->inode_table);
	printf("Free blocks  %d\n", bg->free_blocks_count);
	printf("Free inodes  %d\n", bg->free_inodes_count);
	printf("Used dirs    %d\n", bg->used_dirs_count);*/
	
	buffer* bbm = buffer_read(dev, bg->block_bitmap);
	buffer* ibm = buffer_read(dev, bg->inode_bitmap);

	uint32_t* block_bitmap = malloc(BLOCK_SIZE);
	uint32_t* inode_bitmap = malloc(BLOCK_SIZE);

	for (int i = 0; i < BLOCK_SIZE/4; i += 4) 
		block_bitmap[i] = byte_order(*(uint32_t*)((uint32_t)bbm->data + i));
	for (int i = 0; i < BLOCK_SIZE/4; i += 4) 
		inode_bitmap[i] = byte_order(*(uint32_t*)((uint32_t)ibm->data + i));


	printf("First free BBM: %d\n", ext_first_free(block_bitmap, \
		s->blocks_per_group / 8));
	printf("First free IBM: %d\n", ext_first_free(inode_bitmap, \
		s->blocks_per_group / 8));

	//for (int i =0; i < 20; i++) {
//	buffer* c = buffer_read(dev, 18);
	//	int ticks = get_ticks();
	//while(ticks + 20 > get_ticks());
	//buffer_dump(c);
	//}

	return bg;
}

inode* inode_dump(int i) {
	inode* in = ext2_inode(1, i);

	printf("Mode   %x\t", in->mode);			// Format of the file, and access rights
	printf("UID    %x\t", in->uid);			// User id associated with file
//	printf("Size   %d\n", in->size);			// Size of file in bytes
/*	printf("Atime  %x\n", in->atime);			// Last access time, POSIX
	printf("Ctime  %x\n", in->ctime);			// Creation time
	printf("Mtime  %x\n", in->mtime);			// Last modified time
	printf("Dtime  %x\n", in->dtime);			// Deletion time
	printf("Group  %x\n", in->gid);			// POSIX group access
	printf("#Links %x\n", in->links_count);	// How many links
	printf("Sector %d\n", in->blocks);		// # of 512-bytes blocks reserved to contain the data
	printf("Flags  %x\n", in->flags);			// EXT2 behavior*/
//	printf("%x\n", in->osdl);			// OS dependent value
	printf("Block1 %d\n", in->block[0]);		// Block pointers. Last 3 are indirect

	return in;
//	printf("%x\n", i->generation);	// File version
//	printf("%x\n", i->file_acl);		// Block # containing extended attributes
//	printf("%x\n", i->dir_acl);
//	printf("%x\n", i->faddr);			// Location of file fragment
//	printf("%x\n", i->osd2[3]);
}


// For debugging purposes
void sb_dump(struct superblock_s* sb) {
	printf("Inodes Count %d\n", sb->inodes_count);
	printf("Blocks Count %d\n", sb->blocks_count);
	//printf("RBlock Count %d\n", sb->r_blocks_count);
	//printf("FBlock Count %d\n", sb->free_blocks_count); 	;
	//printf("FInode Count %d\n", sb->free_inodes_count);
	printf("First Data B %d\n", sb->first_data_block);
	printf("Block Sz     %d\n", 1024 << sb->log_block_size);
	printf("Fragment Sz  %d\n", 1024 << sb->log_frag_size);
	printf("Block/Group  %d\n", sb->blocks_per_group);
	printf("Frag/Group   %d\n", sb->frags_per_group);
	printf("Inode/Group  %d\n", sb->inodes_per_group);
/*	printf("mtime; %x\n", sb->mtime);
	printf("wtime; %x\n", sb->wtime);
	printf("mnt_co %x\n", sb->mnt_count);
	printf("max_mn %x\n", sb->max_mnt_count);*/
	printf("EXT2 Magic   %x\n", sb->magic);
/*	printf("state; %x\n", sb->state);
	printf("errors %x\n", sb->errors);
	printf("minor_ %x\n", sb->minor_rev_level);
	printf("lastch %x\n", sb->lastcheck);
	printf("checki %x\n", sb->checkinterval);
	printf("creato %x\n", sb->creator_os);
	printf("rev_le %x\n", sb->rev_level);
	printf("def_re %x\n", sb->def_resuid);
	printf("def_re %x\n", sb->def_resgid);*/
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