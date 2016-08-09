//vga.c

#include <vga.h>

char* VGA_MEMORY = 0x000B8000;


int CURRENT_X = 0;
int CURRENT_Y = 0;
int CURRENT_ATTRIB = 0;

int vga_current_x() { return CURRENT_X; }
int vga_current_y() { return CURRENT_Y; }

void vga_setcolor(int attrib) {
	CURRENT_ATTRIB = attrib;
}

void vga_clear() {
	char* vga_address = VGA_MEMORY;

	const long size = 80 * 25;
	for (long i = 0; i < size; i++ ) { 
		*vga_address++ = 0; // character value
		*vga_address++ = CURRENT_ATTRIB; // color value
	}
}

// kernel level putc, designate x and y position
void vga_kputc(char c, int x, int y) {
	char *vga_address = VGA_MEMORY + (x + y * 160);
	*vga_address = c | (CURRENT_ATTRIB << 8);
}

void vga_puts(char* s) {
	int i = 0;
	while (*s != 0) {
		vga_putc(*s);
		*s++;

	}
}

// Automatically update text position, used in vga_puts
void vga_putc(char c) {
	if (c == '\n') {
		CURRENT_Y += 1;
		CURRENT_X = 0;
		return;
	}
	vga_kputc(c, CURRENT_X, CURRENT_Y);
	CURRENT_X += 2;
	if (CURRENT_X >= 160) {
		CURRENT_X = 0;
		CURRENT_Y += 1;
	}

}