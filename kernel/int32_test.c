
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
