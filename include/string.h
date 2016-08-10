/*
string.h
*/
#include <types.h>
#ifndef __baremetal_string__
#define __baremetal_string__

extern int strlen();
extern char* strcat( char* destination, const char* source );
extern char* strncat( char * destination, const char* source, uint16_t n);

#endif