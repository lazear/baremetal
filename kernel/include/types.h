/*
types.h
*/

#ifndef __crunchy_types__
#define __crunchy_types__

typedef unsigned char uint8_t;			// 1 byte
typedef unsigned short uint16_t;		// 2 bytes
typedef unsigned long uint32_t;			// 4 bytes
typedef unsigned long long uint64_t;	// 8 bytes

typedef unsigned long size_t;			// 4 bytes

typedef int bool;
#define true	1
#define false	0

#define NULL	((void*) 0)
#define panic(a)	(printf("PANIC: %s (%s:%d:%s)\n", a, __FILE__, __LINE__, __func__))
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

#define dprint(e) (printf("%s: %x\n", #e, e) )


#ifndef KERNEL_VIRT
#define KERNEL_VIRT		0xC0000000
#endif

struct _proc_mmap {
	uint32_t* pd;
	uint32_t base;
	uint32_t brk;
} cp_mmap;




// Convert physical to higher half.
#define P2V(x)	( ((uint32_t) (x) <= KERNEL_VIRT) ? ((uint32_t) (x) + KERNEL_VIRT) : (uint32_t) (x))
#define V2P(x)	( ((uint32_t) (x) >= KERNEL_VIRT) ? ((uint32_t) (x) - KERNEL_VIRT) : (uint32_t) (x))
#endif