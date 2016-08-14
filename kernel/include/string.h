/*
string.h
*/
#include <types.h>
#ifndef __baremetal_string__
#define __baremetal_string__

extern size_t strlen();
extern char* strcat( char* destination, const char* source );
extern char* strncat( char * destination, const char* source, size_t n);

#endif