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

#define EXT2_BOOT		0			// Block 0 is bootblock
#define EXT2_SUPER		1			// Block 1 is superblock
#define EXT2_MAGIC		0x0000EF53

typedef struct superblock_s {
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
} superblock;

typedef struct block_group_descriptor_s {
	uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks_count;
	uint16_t free_inodes_count;
	uint16_t used_dirs_count;
	uint16_t pad[7];
} block_group_descriptor;


/*
maximum value of inode.block[index] is inode.blocks / (2 << log_block_size)

Locating an inode:
block group = (inode - 1) / s_inodes_per_group

inside block:
local inode index = (inode - 1) % s_inodes_per_group

containing block = (index * INODE_SIZE) / BLOCK_SIZE
*/
typedef struct inode_s {
	uint16_t mode;			// Format of the file, and access rights
	uint16_t uid;			// User id associated with file
	uint32_t size;			// Size of file in bytes
	uint32_t atime;			// Last access time, POSIX
	uint32_t ctime;			// Creation time
	uint32_t mtime;			// Last modified time
	uint32_t dtime;			// Deletion time
	uint16_t gid;			// POSIX group access
	uint16_t links_count;	// How many links
	uint32_t blocks;		// # of 512-bytes blocks reserved to contain the data
	uint32_t flags;			// EXT2 behavior
	uint32_t osdl;			// OS dependent value
	uint32_t block[15];		// Block pointers. Last 3 are indirect
	uint32_t generation;	// File version
	uint32_t file_acl;		// Block # containing extended attributes
	uint32_t dir_acl;
	uint32_t faddr;			// Location of file fragment
	uint32_t osd2[3];
} inode;

#define INODE_SIZE (sizeof(inode))

typedef struct dirent_s {
	uint32_t inode;			// Inode
	uint16_t rec_len;		// Total size of entry, including all fields
	uint8_t name_len;		// Name length, least significant 8 bits
	uint8_t file_type;		// Type indicator
	uint8_t name[];
} dirent;

/* IMPORTANT: Inode addresses start at 1 */


#endif