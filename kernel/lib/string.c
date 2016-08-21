/*
string.c
Michael Lazear, 2007-2016

Implementation of string library for baremetal project
*/

#include <string.h>
#include <types.h>

// Returns length of a null-terminated string
size_t strlen( char* s ) {
	char* p = s;
	uint32_t i = 0;
	while (*p++ != 0 ) i++;
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


int strncmp(char* s1, char* s2, size_t n) {
	for (size_t i = 0; i < n && *s1 == *s2; s1++, s2++, i++)
		if (*s1 == '\0')
			return 0;
	return ( *(unsigned char*)s1 - *(unsigned char*)s2 );
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


char* strtok(char* s, const char* delim) {
	char* b = NULL;
	static char* ptr, d = NULL;


	if (s) 
		ptr = s;

	if (!ptr && !s)
		return NULL;
	for (int i = 0; i < strlen(delim); i++) {
		b = ptr;
		ptr = strchr(ptr, delim[i]);
		if (!ptr && !b) 
			return NULL;
		*--ptr = '\0';
		ptr++;
		return b;
	}
	return NULL;
}