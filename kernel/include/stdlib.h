/*
stdlib.h
*/

#ifndef __crunchy_stdlib__
#define __crunchy_stdlib__

#include <types.h>

extern char* itoa(uint32_t num, char* buffer, int base);
extern int atoi(char* s);
extern double atof(char* s);
extern char* ftoa(double num, char* str);

extern int dlog10(double v);
extern int log10(int v);
extern uint32_t abs(int x);
extern int pow(int n, int x);

extern void free(void* ptr);
extern void* sbrk(size_t n);
extern void* malloc(size_t n);

#endif