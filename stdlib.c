/*
stdlib.c
Michael Lazear, 2007-2016

Implementation of stdlib for baremetal
*/

#include <types.h>
#include <stdlib.h>


struct  KHEAPBLOCK {
	struct KHEAPBLOCK	*next;
	uint32_t	size;
	uint8_t		used;
} __attribute__((__packed__));

struct  KHEAPBM {
	struct KHEAPBLOCK 	*block;
} __attribute__((__packed__));

// QUICK and DIRTY alloc function
void *HEAP = 0x00200000;
int INDEX = 0;
void *alloc(uint32_t bytes) {
	
	void* ptr = HEAP + INDEX;
	INDEX+=bytes;
	return ptr;

}

void k_heap_init(struct KHEAPBM *master) {
	//master->block = 0;

	//struct KHEAPBLOCK* kb = (struct KHEAPBLOCK*) HEAP;
	struct KHEAPBLOCK* kb = alloc(sizeof(struct KHEAPBLOCK));
	kb->size= 4096;

	char buf[8];
	itoa(kb, buf, 16);

	vga_puts(buf);

}


// string to double
double atof();

// string to integer
int atoi(char* s) {
	int num = 0;
	int sign = 1;

	if (s[0] == '-')
		sign = -1;

	for (int i = 0; i < strlen(s) && s[i] != '\0'; i++) {
		if (IS_NUMERIC_CHAR(s[i]))
			num = num*10 + (s[i] - '0');
	}
	return sign*num;

}

// integer to string
char* itoa(int num, char* buffer, int base) {
	int i = 0;
	int sign = 1;

	if (num == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}
	if (num < 0) sign = 0;

	// go in reverse order
	while (num != 0) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'a' : remainder + '0';
		num = num / base;
	}

	if (sign == 0) buffer[i++] = '-';
	buffer[i] = '\0';

	return strrev(buffer);
}

