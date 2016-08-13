
#ifndef __baremetal_types__
#define __baremetal_types__

typedef unsigned char uint8_t;			// 1 byte
typedef unsigned short uint16_t;		// 2 bytes
typedef unsigned long uint32_t;			// 4 bytes
typedef unsigned long long uint64_t;	// 8 bytes

typedef unsigned long size_t;			// 4 bytes

typedef struct k_stream_object {
	char* data;
	int size;
	int offset;
	int status : 1;		// 1 is good
} STREAM;

/*
Allows asynchronous (is that the right term?) reading and writing
i.e. the writer and reader have their own offsets, eliminating the need
to call fseek() on the stream.
*/
typedef struct k_async_stream_object {
	char* data;
	int size;
	int write_offset;
	int read_offset;
} ASYNC_STREAM;


typedef int bool;
#define true	1
#define false	0

#define NULL	((void*) 0)


#endif