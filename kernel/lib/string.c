/*
string.c
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

Implementation of string library for crunchy project
*/

#include <string.h>
#include <types.h>

// Returns length of a null-terminated string
size_t strlen( const char* s ) {
	int i = 0;
	while(*s) {
		i++;
		s++;
	}
	return i;
}


// Appends a copy of the source string to the destination string
char* strncat( char* destination, const char* source, size_t n ) {
	size_t length = strlen(destination);
	int i;
	for (i = 0; i < n && source[i] != '\0'; i++)
		destination[length+i] = source[i];
	destination[length+i] = '\0';
	return destination;
}

// wrapper for strncat
char* strcat(char *destination, const char* source) {
	return strncat(destination, source, strlen(source));
}


// Copy first n characters of src to destination
char* strncpy(char *dest, const char *src, uint16_t n) {
    uint16_t i;

   for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';

   return dest;
}

// Copy all of str to dest
char* strcpy(char *dest, const char *src) {
	return strncpy(dest, src, strlen(src));
}


int strncmp(const char *s1, const char *s2, size_t n)
{
  //   for ( ; n > 0; s1++, s2++, --n)
		// if (*s1 != *s2)
		// 	return ( (int) *s1 - (int) *s2);
		// else if (*s1 == '\0')
		// 	return 0;


	for (int i = 0; i < n; i++)
		if (s1[i] != s2[i])
			return ((unsigned char) s1[i] - (unsigned char) s2[i]);
    return 0;
}


int strcmp(char *s1, char* s2) {
	return strncmp(s1, s2, strlen(s1));
}


char* strchr(const char* s, int c) {
	while (*s != '\0')
		if (*s++ == c) return (char*) s;
	return NULL;
}

char* strdup(const char* s) {
	return strcpy((char*) malloc(strlen(s)), s);
}

/*
In place string reverse
*/
char* strrev(char* s) {
	int length = strlen(s) - 1;
	for (int i = 0; i <= length/2; i++) {
		char temp = s[i];
		s[i] = s[length-i];
		s[length-i] = temp;
	}
	return s;
}


void* memcpy(void *s1, const void *s2, uint32_t n) {
	uint8_t* src = (uint8_t*) s2;
	uint8_t* dest = (uint8_t*) s1;

	for (int i = 0; i < n; i++)
		dest[i] = src[i];
	return s1; 
}

void* memset(void *s, int c, size_t n) {
	uint8_t* dest = (uint8_t*) s;
	for (size_t i = 0; i < n; i++)
		dest[i] = c;
	return s;
}

void* memsetw(void *s, int c, size_t n) {
	uint16_t* dest = (uint16_t*) s;
	for (size_t i = 0; i < n; i++) *dest++ = c;
}

void* memmove(void *s1, const void* s2, size_t n) {
	char* dest 	= (char*) s1;
	char* src 	= (char*) s2;
	char* temp = (char*) malloc(n);

	for (int i = 0; i < n; i++)
		temp[i] = src[i];
	for (int i = 0; i < n; i++)
		dest[i] = temp[i];

	free(temp);
	return s1;
}

void* memchr(const void* s, int c, size_t n) {
	uint8_t* b = s;
	while (n--)
		if (*b++ == c)
			return b;
	return NULL;
}

void* memrchr(const void* s, int c, size_t n) {
	return strrev(memchr(strrev(s), c, n));
}

int memcmp(const uint8_t* s1, const uint8_t* s2, size_t n) {

	while (*s1 == *s2 && n--) {
		s1++;
		s2++;
	}
	return ( *(uint8_t*)s1 - *(uint8_t*)s2 );
}

char* strpbrk(char* s, const char* delim) {
	while(*s++) 
		for (int q = 0; q < strlen(delim); q++)
			if (*s == delim[q]) 
				return s;
	return NULL;
}

char* strstr(char* s1, const char* s2) {
	while(*s1++) 
		if (strncmp(s1, s2, strlen(s2)) == 0)
			return s1;
	return NULL;
}

int strspn(const char* s1, const char* s2) {
	for (int i = 0; i < strlen(s1); i++)
		if (strchr(s2, s1[i]) == NULL)
			return i;
	return strlen(s1);
}

/* new and improved strtok, using strspn and strpbrk */
char* strtok(char* s, const char* delim) {
	char* tmp = NULL;
	static char* ptr;

	if (s) 
		ptr = s;
	if (!ptr && !s)
		return NULL;
	ptr = (unsigned int) ptr + strspn(ptr, delim);
	char* t = strpbrk(ptr, delim);

	if (t) {
		*t++ = 0;
		tmp = ptr;
		ptr = t;
	} else {
		tmp = ptr;
		ptr = NULL;
	}
	return tmp;
}
