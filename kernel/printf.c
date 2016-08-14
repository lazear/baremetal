
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
	char buf[20];
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
	return count;
}

