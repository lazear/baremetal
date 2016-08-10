/*
Michael Lazear, (C) 2007-2016

kernel.c
*/

#include <kernel.h>
#include <vga.h>
#include <x86.h>


int ticks = 0;


void timer(struct regs *r) {
	ticks++;
	
}

void itoa(char *buf, int base, int d, signed int length, char flags)
{
   char *p = buf;
   char *p1, *p2;
   unsigned long ud = d;
   int divisor = 10;

   /* If %d is specified and D is minus, put `-' in the head. */
   if(base == 10 && d < 0)
   {
      *p++ = '-';
      buf++;
      ud = -d;
   }
   else if( base == 16 )
      divisor = 16;
   else if( base < 16 )
	   divisor = base;

   /* Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;
		// funky if
		*p++ = (remainder < 10) ? remainder + '0' : remainder + flags - 10;
		length--;
	}
	while(ud /= divisor);

   
	while(length > 0)
	{
		*p++ = '0';
		length--;
	}

	/* Terminate BUF. */
	*p = 0;

	/* Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while(p1 < p2)
	{
		char tmp = *p1;

		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


void test(struct regs *r) {

}

//We enter into kernel initialize with the GDT and IDT already loaded, and interrupts enabled
void kernel_initialize() {

	vga_setcolor(VGA_LIGHTGREY);
	vga_clear();

	vga_puts("baremetal!\n");
	irq_install_handler(1, test);
	irq_install_handler(0, timer);
	char *s;
	int d = &kernel_initialize; // position of the kernel in memory

	itoa(s, 16, d, 8, 'a');
	vga_puts("Kernel loaded to: ");
	vga_puts(s);
	vga_pretty("Red", 0x4);
	vga_pretty("Blue", 0x9);
	vga_pretty("Green", 0x2);
	vga_pretty("Cyan", VGA_CYAN);
	vga_pretty("Grey", VGA_LIGHTGREY);
	vga_pretty("Pink\n", VGA_LIGHTMAGENTA);

	for(;;);
}

