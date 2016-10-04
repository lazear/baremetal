/*
printf.c
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

Kernel level printf. Should be removed once userspace is up and running
*/


#include <types.h>
#include <vga.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>


#define PREFIX			1
#define ALWAYS_SIGN		2
#define PAD 			4
#define LONG			8
#define NOTNUM			0x10

char* ltoa(uint64_t num, char* buffer, int base) {
	int i = 0;
	// go in reverse order
	while (num != 0) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}

//	if (sign == 0) buffer[i++] = '-';

	buffer[i] = '\0';
	strrev(buffer);
	return buffer;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {

	char* buf = malloc(size);
	memset(str, 0, size);
	memset(buf, 0, size);

	size_t n = 0;
	size_t pad = 0;
	int flags = 0;

	while(*format && n < size) {
		switch(*format) {
			case '%': {
				format++;

next_format:
				switch(*format) {
					case '%': {
						buf[0] = '%';
						buf[1] = '\0';
						break;
					}
					case '#': {
						flags |= PREFIX;
						format++;
						goto next_format;
					}
					case '+': {
						flags |= ALWAYS_SIGN;
						format++;
						goto next_format;
					}
					case 'x': {
						itoa(va_arg(ap, int), buf, 16);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
							str[n++] = 'x';
						}
						break;
					}
					case 'd': case 'i': {
						int d = va_arg(ap, int);
						if ((flags & ALWAYS_SIGN) && (n+1 < size)) 
							str[n++] = (d > 0) ? '+' : '-';
						sitoa(d, buf, 10);
				
						break;
					}
					case 'u': {
						int d = va_arg(ap, int);
						itoa(d, buf, 10);
						break;
					}
					case 'l': {
						uint64_t d = va_arg(ap, uint64_t);
						flags |= LONG;
						ltoa(d, buf, 16);
						break;
					}
					case 'o': {
						itoa(va_arg(ap, int), buf, 8);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
						}
						break;
					}
					case 'b': {
						itoa(va_arg(ap, int), buf, 2);
						break;
					}
					case 's': {
						char* tmp = va_arg(ap, char*);
						strcpy(buf, tmp);
						flags |= NOTNUM;
						break;
					}
					case 'c': {
						buf[0] = va_arg(ap, char);
						buf[1] = '\0';
						flags |= NOTNUM;
						break;
					}
					case 'w': {
						flags |= PAD;
						buf[0] = ' ';
						break;
					}
					default: {
						/* Parse out padding information */
						pad = 0;
						while(isdigit(*format)) {
							flags |= PAD;
							pad *= 10;
							pad += *format - '0';
							format++;
						}
						goto next_format;
					}

				}
				if ((flags & PAD) && (pad > strlen(buf))) {
					for (int i = 0; (i < (pad - strlen(buf))) && ((i+n) < size); i++)
						str[n++] = (flags & NOTNUM) ? ' ' : '0';
				}
				for (int i = 0; (i < strlen(buf)) && ((i+n) < size); i++) 	
					str[n++] = buf[i];		
				memset(buf, 0, strlen(buf));
				break;
			}
			default: {
				str[n++] = *format;
				flags = 0;
				break;
			}
		}
		format++;
	}

	free(buf);
	return n;
}



int printf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char buf[256];
	int i = vsnprintf(buf, 256, fmt, ap);
	va_end(ap);

	vga_puts(buf);
	return i;
}
