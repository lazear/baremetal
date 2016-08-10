/*
stdlib.h
*/

#ifndef __baremetal_stdlib__
#define __baremetal_stdlib__

#define IS_NUMERIC_CHAR(c)	((c >= '0') && (c <= '9'))

extern char* itoa(int num, char* buffer, int base);
extern int atoi(char* s);

#endif