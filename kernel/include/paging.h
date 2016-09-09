
#include <types.h>

#ifndef __baremetal_paging__
#define __baremetal_paging__

#define PF_SUPERVISOR	0x0
#define PF_PRESENT		0x1
#define PF_RW			0x2
#define PF_USER			0x4

/* 
assembly functions
*/
extern void k_paging_enable();
extern void k_paging_load_directory(uint32_t* dir);
extern uint32_t* k_read_cr3();


extern int k_paging_unmap(uint32_t virt);
extern void k_paging_map(uint32_t phys, uint32_t virt, uint8_t flags);

extern void k_paging_init(uint32_t* dir_addr);
extern uint32_t* k_virt_to_phys(uint32_t virt);

extern uint32_t* k_phys_to_virt(uint32_t phys);
extern uint32_t* k_create_pagedir(uint32_t virt, uint32_t numpages, int flags) ;


#endif