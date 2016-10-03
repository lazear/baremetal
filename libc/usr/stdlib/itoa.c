/*
stdlib.c
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

Implementation of stdlib for crunchy
*/

#include <types.h>
#include <ctype.h>
#include <stdlib.h>

// unsigned integer to string
char* itoa(uint32_t num, char* buffer, int base) {
	int i = 0;
	//num = abs(num);
	int len = 8;

	if (base == 2)
		len = 32;
	
	if (num == 0 && base == 2) {
		while(i < len)
			buffer[i++] = '0';
		buffer[i] = '\0';
		return buffer;
	}

	// go in reverse order
	while (num != 0 && len--) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}

	while(len-- && base != 10)
		buffer[i++] = '0';

	buffer[i] = '\0';

	return strrev(buffer);
}
