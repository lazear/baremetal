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
#include <stdarg.h>

extern int printf(const char* fmt, ...);

char* format( const char *fmt, ... ) {
	va_list args;
	va_start(args, fmt);
	int count;
	// buffer is for itoa/sitoa conversions
	char* buffer = malloc(16 * sizeof(char));
	char *formatted = malloc(strlen(fmt) * sizeof(char) + 16);

	while ( *fmt != 0 ) {
		switch (*fmt) {
			case '%':
				fmt++;
				switch( *fmt ) {
					case 'd':
					case 'i':
						sitoa(va_arg(args, int), buffer, 10);
						vga_puts(buffer);
						strcat(formatted, buffer);
						memset(buffer, 0, strlen(buffer));
					case 'x':
						itoa(va_arg(args, int), buffer, 16);
						strcat(formatted, buffer);
						memset(buffer, 0, strlen(buffer));

				}
			default:
				*formatted = *fmt;
				formatted++;
				fmt++;

		}
		vga_putc(*formatted);
	}
	return formatted;
}

int printf( const char *fmt, ... ) {
	va_list args;
	va_start(args, fmt);
	int count;
	char *buf = malloc(32 * sizeof(char));
	//char buf[32];
	memset(buf, 0, 32);
	int len = 8;
	
	while ( *fmt != 0 )
	{
		switch ( *fmt ){
			case '%':
				fmt++;
				switch ( *fmt )
				{
				case 'd':
				case 'i':
					sitoa(va_arg(args, int), buf, 10);
					//sitoa(buf, 10, va_arg(args, int));
					vga_puts(buf);
					break;
				case 'u':	// Unsigned
					sitoa(va_arg(args, int), buf, 16);
					vga_puts(buf);
					break;		
				case 'b':	// Byte in hex
					itoa(va_arg(args, uint32_t), buf, 2);
					vga_puts(buf);
					break;
				case 'w':	// word
					itoa(va_arg(args, uint32_t), buf, 16);
					vga_puts(buf);
					break;
				case 'x':	// hex
					itoa(va_arg(args, uint32_t), buf, 16);
					//itoa(buf, 16, va_arg(args, uint32_t));
					vga_puts(buf);
					break;
				case 'X':	// HEX
					itoa(va_arg(args, uint32_t), buf, 16);
					vga_puts(buf);
					break;
				case 'o':	// Octal
					itoa(va_arg(args, uint32_t), buf, 8);
					vga_puts(buf);
					break;
				case 's':	// string (char*)
					vga_puts(va_arg(args, char*));
					break;
				case 'c':	// char
					vga_putc(va_arg(args, char));
					break;
				case 'e':
					vga_puts(ftoa(va_arg(args, double), buf));
							
				}
				fmt++;
				break;
			default:
				vga_putc(*fmt);
				fmt++;
		}
		count++;
	}
	vga_scroll();
	va_end(args);
	free(buf);
	return count;
}

