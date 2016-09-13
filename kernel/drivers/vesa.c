
#include <types.h>

typedef struct _gfx_context {
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint32_t framebuffer;
} gfx_context;

#define RGB(r,g,b) (((r&0xFF)<<16) | ((g&0xFF)<<8) | (b & 0xFF))

static void putpixel(unsigned char* screen, int x,int y, int color) {
	unsigned where = x*3 + y*768*4;
	screen[where] = color & 255;              // BLUE
	screen[where + 1] = (color >> 8) & 255;   // GREEN
	screen[where + 2] = (color >> 16) & 255;  // RED
}


extern uint32_t _init_pd[];
void vesa_init() {

}