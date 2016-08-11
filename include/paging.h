
#ifndef __baremetal_paging__
#define __baremetal_paging__

#define PF_SUPERVISOR	0x0
#define PF_PRESENT		0x1
#define PF_RW			0x2
#define PF_USER			0x4
#define GET_PGE(m)		(((m)<<12)>>22)


typedef struct{
	unsigned int present	: 1;
	unsigned int rw			: 1;
	unsigned int user		: 1;
	unsigned int accessed	: 1;
	unsigned int dirty		: 1;
	unsigned int unused		: 7;
	unsigned int frame		: 20;
} page_t;

#endif