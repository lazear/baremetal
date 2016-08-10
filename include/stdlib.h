/*
stdlib.h
*/

#ifndef __baremetal_stdlib__
#define __baremetal_stdlib__


extern void itoa(char *buf, int base, int d, signed int length, char flags);
extern int atoi(char* s);

#endif