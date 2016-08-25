/*
ext2.h
================================================================================
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
================================================================================
*/

#include <types.h>

#ifndef __baremetal_ext2__
#define __baremetal_ext2__

#define EXT2_MAGIC		0xEF53


struct superblock_s {
	uint32_t inodes_count;			// Total # of inodes
	uint32_t blocks_count;			// Total # of blocks
	uint32_t r_blocks_count;		// # of reserved blocks for superuser
	uint32_t free_blocks_count;	
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;		// 1024 << Log2 block size  = block size
	uint32_t log_frag_size;
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;					// Last mount time, in POSIX time
	uint32_t wtime;					// Last write time, in POSIX time
	uint16_t mnt_count;				// # of mounts since last check
	uint16_t max_mnt_count;			// # of mounts before fsck must be done
	uint16_t magic;					// 0xEF53
	uint16_t state;
	uint16_t errors;
	uint16_t minor_rev_level;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creator_os;
	uint32_t rev_level;
	uint16_t def_resuid;
	uint16_t def_resgid;
};

struct block_group_descriptor_s {
	uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks_count;
	uint16_t free_inodes_count;
	uint16_t used_dirs_count;
	uint16_t pad[7];
};


#endif