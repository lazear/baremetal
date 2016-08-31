/*
vfs.c
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

Begin work on implementing virtual filesystem. Use a *NIX-ish approach, with 
everything mounted under "/" (root)
*/

#include <types.h>
#include <ext2.h>

#define VFS_DIR		2
#define VFS_FILE	1

typedef struct vfs_entry_s {
	int dev;
	int inode;
	int type;
	char* name;
	struct vfs_entry_s* parent;
} vfs_entry;

vfs_entry* root = NULL;

void vfs_init() {
	root = malloc(sizeof(vfs_entry));
	root->dev 	= 1;
	root->inode = 2;
	root->type 	= VFS_DIR;
	root->parent = NULL;
	strcpy(root->name, "/");
}

void vfs_traverse(char* name) {
	dirent* d = ext2_open_dir(2);
	do{
	//	d->name[d->name_len] = '\0';
		printf("%s%s\n", name, d->name);

		d = (dirent*)((uint32_t) d + d->rec_len);
	} while(d->inode);
}