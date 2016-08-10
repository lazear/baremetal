/*
string.c
Michael Lazear, 2007-2016

Implementation of string library for baremetal project
*/

#include <string.h>
#include <types.h>

// Returns length of a null-terminated string
int strlen( char* s ) {
	char* p = s;
	uint32_t i = 0;
	while (*p++ != 0 ) i++;
	return i;
}


// Appends a copy of the source string to the destination string
char* strcat( char* destination, const char* source ) {
	uint16_t length = strlen(destination);
	int i;
	for (i = 0; i < strlen(source); i++)
		destination[length+i] = source[i];
	destination[length+i] = '\0';
	return destination;
}

// Appends a copy of the source string to the destination string
char* strncat( char* destination, const char* source, uint16_t n ) {
	uint16_t length = strlen(destination);
	int i;
	for (i = 0; i < n && source[i] != '\0'; i++)
		destination[length+i] = source[i];
	destination[length+i] = '\0';
	return destination;
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


char* strchr(const char* s, int c) {
	while (*s != '\0')
		if (*s++ == c) return (char*) s;
	return NULL;
}

/*
In-place string reverse. May be dangerous to use.
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
	for (int i = 0; i < n; i++)
		dest[i] = c;
	return s;
}

void* memmove(void *s1, const void* s2, size_t n) {
	char* dest 	= (char*) s1;
	char* src 	= (char*) s2;
	char temp[n]; // add in malloc

	for (int i = 0; i < n; i++)
		temp[i] = src[i];
	for (int i = 0; i < n; i++)
		dest[i] = temp[i];

	//add in free(temp)

	return s1;
}