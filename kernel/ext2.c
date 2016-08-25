
#include <ide.h>
#include <ext2.h>
#include <types.h>
#include <assert.h>


/* 	Read superblock from device dev, and check the magic flag.
	Return NULL if not a valid EXT2 partition */
superblock* ext2_superblock(int dev) {
	buffer* b = buffer_read(dev, EXT2_SUPER);
	superblock* sb = (superblock*) b->data;
	
	assert(sb->magic == EXT2_MAGIC);
	if (sb->magic != EXT2_MAGIC)
		return NULL;
	assert(1024 << sb->log_block_size == BLOCK_SIZE);
	return sb;
}

block_group_descriptor* bgd(int dev) {
	buffer* b = buffer_read(dev, EXT2_SUPER + 1);
	block_group_descriptor* bg = (block_group_descriptor*) b->data;
	
	printf("Block bitmap %d\n", bg->block_bitmap);
	printf("Inode bitmap %d\n", bg->inode_bitmap);
	printf("Inode table  %d\n", bg->inode_table);
	printf("Free blocks  %d\n", bg->free_blocks_count);
	printf("Free inodes  %d\n", bg->free_inodes_count);
	printf("Used dirs    %d\n", bg->used_dirs_count);
	
	//for (int i =0; i < 20; i++) {
//	buffer* c = buffer_read(dev, 18);
	//	int ticks = get_ticks();
	//while(ticks + 20 > get_ticks());
	//buffer_dump(c);
	//}

	return bg;
}

inode* inode_dump(superblock* s, int i) {
	int block_group = (i - 1) / s->inodes_per_group;
	int index 		= (i - 1) % s->inodes_per_group;
	int block 		= (index * INODE_SIZE) / BLOCK_SIZE;
	printf("%d, %d, %d\n", block_group, index, block);
}


// For debugging purposes
void sb_dump(struct superblock_s* sb) {
	printf("Inodes Count %d\n", sb->inodes_count);
	printf("Blocks Count %d\n", sb->blocks_count);
	printf("RBlock Count %d\n", sb->r_blocks_count);
	printf("FBlock Count %d\n", sb->free_blocks_count); 	;
	printf("FInode Count %d\n", sb->free_inodes_count);
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