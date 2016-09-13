/*
mutex.h
*/

#include <types.h>

#ifndef __crunchy_mutex__
#define __crunchy_mutex__

typedef struct mutex_struct {
	unsigned char lock : 1;
} __attribute__((packed)) mutex;


extern void acquire(mutex *m);
extern void release(mutex *m);

extern void pushcli();
extern void popcli();

#endif