/*
stdio.h
*/

#ifndef __crunchy_stdio__
#define __crunchy_stdio__

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

#endif