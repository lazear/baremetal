/*
stdlib.h
*/

#ifndef __baremetal_stdlib__
#define __baremetal_stdlib__

#define IS_NUMERIC_CHAR(c)	((c >= '0') && (c <= '9'))

extern char* itoa(uint32_t num, char* buffer, int base);
extern int atoi(char* s);
extern double atof(char* s);
extern char* ftoa(double num);

extern int dlog10(double v);
extern int log10(int v);
extern uint32_t abs(int x);
extern int pow(int n, int x);

#endif