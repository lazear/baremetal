/*
keyboard.c
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
*/

#include <x86.h>
#include <types.h>
#include <stdio.h>
#include <mutex.h>
#include <traps.h>

uint8_t keyboard[] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't','y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 0, '\\', 'z', 
  'x', 'c', 'v', 'b', 'n','m', ',', '.', '/', 0,'*', 0,' ', 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 254, 0, '-', 252, 0, 253, '+', 0, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0,
};
uint8_t keyboard_shift[] =
{
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T','Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':','"', '~', 0, '|', 'Z', 
  'X', 'C', 'V', 'B', 'N','M', '<', '>', '?', 0,'*', 0,' ', 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 254, 0, '-', 252, 0, 253, '+', 0, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0,
};


char *buffer;	// we want to dynamically alloc this
char* linebuffer = 0;

uint32_t idx = 0;	// keep track of where we are
int key_wait = 0;
int shift = 0;


void keyboard_handler(regs_t *r)
{
	uint8_t scancode;
	uint8_t key;
	if (idx >= 0x400)
		idx = 0;

    scancode = inb(0x60);
  	uint8_t  data = inb(0x60);

    if (scancode & 0x80)
	{
		scancode &= ~0x80;		/* Get rid of key-release code */
		if (scancode == 0x3a)	/* Caps lock */
		{
			switch (shift)
			{
			case 0:
				shift = 1;
				break;
			default:
				shift = 0;
				break;
			}
			while(1)
				if (!(inb(0x64) & 2)) break;
			outb(0x60, 0xed);			/* Write command */
			outb(0x60, (shift << 2));	/* Caps lock light */
		}
	}
    else 
	{
		if (shift)
			key = keyboard_shift[scancode];
		else
			key = keyboard[scancode];

			vga_putc(key);
			if (key == '\n')	// line return
			{
				strcpy(linebuffer, buffer, (idx < 0x100) ? idx : 0x100);
				idx = 0;
				
			} 
			else if (key == '\b')	// backspace
			{
				if (idx)
					buffer[idx--] = 0;
			}
			else
				buffer[idx++] = key;	// add 'key' to the input buffer

	}

}

char* gets() {
	while(!*linebuffer);
	char* tmp = strdup(linebuffer);
	memset(linebuffer, 0, 0x100);
	return tmp;
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
	buffer = (char*)malloc(0x400);		// allocate and zero the buffer
	linebuffer = malloc(0x100);
	pic_enable(IRQ_KBD);
}
