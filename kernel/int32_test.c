
#include <types.h>

// define our structure
typedef struct __attribute__ ((packed)) {
	unsigned short di, si, bp, sp, bx, dx, cx, ax;
	unsigned short gs, fs, es, ds, eflags;
} regs16_t;

// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);

// int32 test
void int32_test()
{
	int y;
	regs16_t regs;
	
	//		;mov ax, 0x4F02
	//	;mov bx, 0x431A
	// switch to 320x200x256 graphics mode
	
	// 1024*768*16M

	regs.ax = 0x4F02;
	regs.bx = 0x4118;
	int32(0x10, &regs);


}

	char* VID = 0xFD000000;
extern char font[];

#define RGB(r,g,b) (((r&0xFF)<<16) | ((g&0xFF)<<8) | (b & 0xFF))
/* only valid for 800x600x16M */
static void putpixel(unsigned char* screen, int x,int y, int color) {
    unsigned where = x*3 + y*768*4;
    screen[where] = color & 255;              // BLUE
    screen[where + 1] = (color >> 8) & 255;   // GREEN
    screen[where + 2] = (color >> 16) & 255;  // RED
}


void get_font(unsigned char* screen, int c, int x, int y) {
	for (int i = 0; i < 8; i++) {		// row by row
		for(int q = 0; q < 8; q++) {
			if (font[c*8 + i] & (1<<q))
				putpixel(screen, x+q, y+i, RGB(0, 255, 0));
		}
	}

}

void fancy(char* s, int x, int y) {
	for (int i = 0; i < strlen(s); i++) {
		get_font(VID, s[i], x + (i*9), y);
	}
}

extern uint32_t _init_pd[];
void vesa_init() {

	for (int i =0; i < 0x1000000; i+=0x1000) 
		_paging_map(&_init_pd, i+0xFD000000, i+0xFD000000, 0x3);

	int32_test();

	
	for (int i = 0; i < 40; i++) {
		putpixel(VID, 5, i, 0xFFFFFFFF);
		putpixel(VID, 100, i, 0xFFFFFFFF);
	
	}
	for (int i = 0; i < 100; i++) {
		putpixel(VID, i, 5, RGB(0, 0xFF, 0));
		putpixel(VID, i, 40, RGB(0, 0xFF, 0));
	}


	printf("%x", font);
	fancy("0xDEADBEEF!", 11, 11);
	fancy("BAREMETAL!", 11, 19);
}