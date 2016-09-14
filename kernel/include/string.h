/*
string.h
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
Function declarations for string library
*/

#include <types.h>
#ifndef __crunchy_string__
#define __crunchy_string__

extern size_t strlen();
extern char* strcat( char* destination, const char* source );
extern char* strncat( char * destination, const char* source, size_t n);
extern char* strncpy(char *dest, const char *src, uint16_t n);
extern char* strcpy(char *dest, const char *src);
extern int strncmp(char* s1, char* s2, size_t n); 
extern int strcmp(char *s1, char* s2);
extern char* strchr(const char* s, int c);
extern char* strdup(const char* s);
extern char* strrev(char* s); 

extern void* memcpy(void *s1, const void *s2, uint32_t n);
extern void* memset(void *s, int c, size_t n);
extern void* memsetw(void *s, int c, size_t n);
extern void* memmove(void *s1, const void* s2, size_t n); 
extern void* memchr(const void* s, int c, size_t n);
extern void* memrchr(const void* s, int c, size_t n); 
extern int memcmp(const uint8_t* s1, const uint8_t* s2, size_t n);
extern char* strpbrk(char* s, const char* delim);
extern char* strstr(char* s1, const char* s2); 
extern char* strtok(char* s, const char* delim); 


#endif