/*
mutex.c
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

#include <types.h>
#include <mutex.h>
#include <smp.h>
int ncli = 0;

void pushcli() {

	asm volatile("cli");
	ncli++;
}

void popcli() {

	if(--ncli < 0)
		panic("popcli without push");
	if(ncli == 0) 
		asm volatile("sti");


}	


// extern void __acquire(mutex *);
// extern void __release(mutex *);

// void acquire(mutex* m) {
// 	pushcli();
// 	__acquire(m);
// 	//m->holder =  __builtin_return_address(0) ;
// 	//m->cpu = cpu->id;

// 	//asm volatile("mov (%%ebp+4), %0" : "=r"(eip));
// }

// void release(mutex* m) {
// 	m->cpu = -1;
// 	popcli();
// 	__release(m);
// }

mutex ml = {.lock = 0};


int nocli() { return ncli; }

void mutex_test() {


	printf("Begin mutex test\n");
	acquire(&ml);
//	ml.cpu = cpu->id;

	release(&ml);
	printf("release: %d\n", ml.lock);
}

