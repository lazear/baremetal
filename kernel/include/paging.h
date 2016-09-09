/*
paging.h
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

#include <types.h>

#ifndef __baremetal_paging__
#define __baremetal_paging__

#define PF_PRESENT		0x1
#define PF_RW			0x2
#define PF_USER			0x4
#define PF_ACCESSED		0x10	// Bit 5 - PDE/PTE was used for translation
#define PF_DIRTY		0x20	// Bit 6 - PTE only

/* 
assembly functions
*/
extern void k_paging_enable();
extern void k_paging_load_directory(uint32_t* dir);

extern int k_paging_unmap(uint32_t virt);
extern void k_paging_map(uint32_t phys, uint32_t virt, uint8_t flags);

extern void k_paging_init(uint32_t* dir_addr);
extern uint32_t* k_virt_to_phys(uint32_t virt);

extern uint32_t* k_phys_to_virt(uint32_t phys);
extern uint32_t* k_create_pagedir(uint32_t virt, uint32_t numpages, int flags) ;


#endif