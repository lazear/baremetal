/*
mutex.h
*/

#include <types.h>

#ifndef __baremetal_mutex__
#define __baremetal_mutex__

typedef struct mutex_struct {
	int lock : 1;
} __attribute__((packed)) mutex;

#endif