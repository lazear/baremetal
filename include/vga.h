/*
vga.h
*/

#ifndef __baremetal_vga__
#define __baremetal_vga__

// vga_console_object structure allows us to track the current x and y position, as well as color

struct vga_console_object
{
	char x;
	char y;
	char color;
} vga_term;

extern void vga_clear();
extern void vga_puts(char* s);
extern void vga_kputc(char c, int x, int y);
extern void vga_putc(char c);
extern void vga_setcolor(int attrib);

extern int vga_current_x();
extern int vga_current_y();



#endif