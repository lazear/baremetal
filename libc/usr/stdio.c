/*
stream.c
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

Basic stream implementation
*/

#include <types.h>
#include <stdio.h>

/*
Create a new stream of size N bytes and return a pointer to that stream
*/
STREAM* k_new_stream(size_t n) {
	STREAM* stream = malloc(sizeof(STREAM));	// allocate the stream struct to memory
	uint32_t* ptr = malloc(n);					// allocate the stream data location
	
	stream->size = n;
	stream->offset = 0;
	stream->data = ptr;
	return stream;

}

// Writes N bytes to the data stream, starting at the current offset
int k_stream_write(STREAM* stream,  char* msg, size_t n) {
	if (stream->size < n || stream->offset + n > stream->size)
		return -1;

	// pointer arithmetic
	memcpy(((uint32_t) stream->data + stream->offset), msg, n);
	stream->offset += n;

	return 1;
}


// Reads N bytes from the data stream, starting at the current offset
char* k_stream_read(STREAM* stream, size_t n) {
	if (stream->size < n || stream->offset + n > stream->size)
		return NULL;

	char* buffer = malloc(n);
	memcpy(buffer, ((uint32_t) stream->data + stream->offset), n);
}

char fgetc(STREAM* stream) {
	char q = *(char*)((uint32_t) stream->data + stream->offset - 1);
	return q;
}

int k_stream_seek(STREAM* stream, int p) {
	
	if(p < -1)
		return -1;
	if(p >= stream->size)
		return -1;

	stream->offset = p;
	return p;
}

int k_stream_tell(STREAM* stream) {
	return stream->offset;
}

int fputc(STREAM* stream, char c ) {
	if (stream->size < 1 || stream->offset + 1 > stream->size)
		return 0;

	// More crazy pointer math
	*(char*) ((uint32_t) stream->data + stream->offset) = c;

	stream->offset += 1;
	return 1;
}

int fputs(STREAM* stream, char *c) {
	size_t n = strlen(c);
	if (stream->size < n || stream->offset + n > stream->size)
		return -1;

	for (int i = 0; i < n, c[i] != '\0' ; i++)
		fputc(stream, c[i]);
}

/*
fseek() and ftell() wrappers. Use responsibly.
*/

int fseek(STREAM* stream, int p) {
	return k_stream_seek(stream, p);
}

int ftell(STREAM *stream) {
	return k_stream_tell(stream);
}

// fflush - flush buffer to file.
int fflush(STREAM *stream) {
	memset(stream->data, 0, stream->size);
	return fseek(stream, 0);
}