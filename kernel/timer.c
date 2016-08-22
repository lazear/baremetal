/*
timer.c
*/

#include <types.h>
#include <x86.h>
#include <mutex.h>
#include <traps.h>

uint32_t ticks = 0;
char* timer_buf = 0;

void timer(regs_t *r) {
	ticks++;
	if (!(ticks%1)) {
		itoa(ticks, timer_buf, 10);
		vga_kputs(timer_buf, 150, 0);
	}

}

void timer_init() {
	ticks = 0;
	timer_buf = malloc(8);
	memset(timer_buf, 0, 8);
	pic_enable(IRQ_TIMER);
}

uint32_t get_ticks(){
	return ticks;
}




