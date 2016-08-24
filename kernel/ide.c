/*
ide.c
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
Disk buffer cache/LBA read/write support
*/

#include <types.h>
#include <x86.h>
#include <mutex.h>
#include <traps.h>
#include <ide.h>

void sleep(void* channel, mutex* m) {}
void wake(void* channel) {} 

mutex idelock = {.lock = 0};		// spinlock for disk access
buffer* idequeue;					// cache of buffer access
static int present = 0;				// Dev 1 present?

/*
Wait for IDE device to become ready
check =  0, do not check for errors
check != 0, return -1 if error bit set
*/
static int ide_wait(int check) {
	char r;

	// Wait while drive is busy. Once just ready is set, exit the loop
	while (((r = (char)inb(IDE_IO | IDE_CMD)) & (IDE_BSY | IDE_RDY)) != IDE_RDY );

	// Check for errors
	if (check && (r & (IDE_DF | IDE_ERR)) != 0)
		return -1;
	return 0;
}

// Delay 400 ns
static int ide_delay() {
	char ret = 0;
	for (int i = 0; i < 5; i++)
		ret = (char)inb(IDE_ALT);
	return ret;
}

static void ide_op(buffer* b) {
	if (!b)
		panic("ide_op");

	int sector_per_block = BLOCK_SIZE / SECTOR_SIZE;	// 1
	int sector = b->block * sector_per_block;
	ide_wait(0);
	outb(IDE_ALT, 0);	// Generate interrupt
	outb(IDE_IO | IDE_SECN, sector_per_block);	// # of sectors
	outb(IDE_IO | IDE_LOW, LBA_LOW(sector));
	outb(IDE_IO | IDE_MID, LBA_MID(sector));
	outb(IDE_IO | IDE_HIGH, LBA_HIGH(sector));
	// Slave/Master << 4 and last 4 bits
	outb(IDE_IO | IDE_HEAD, 0xE0 | ((b->dev & 1) << 4) | LBA_LAST(sector));	

	if (b->flags & B_DIRTY) {						// Need to write
		outb(IDE_IO | IDE_CMD, IDE_CMD_WRITE);
		outsl(IDE_IO, b->data, BLOCK_SIZE / 4);		// Write in 4 byte chunks (double word)
	} else {										// Read only
		outb(IDE_IO | IDE_CMD, IDE_CMD_READ);
	}
}

/*
Actual reading of data takes place in the interrupt handler
*/
void ide_handler() {
	acquire(&idelock);

	buffer* b = idequeue;
	if(!b) {
		release(&idequeue);
		return;
	}
	idequeue = b->next;

	//if dirty is not set, and no errors, read data
	int stat = ide_wait(1);
	if (!(b->flags & B_DIRTY) && stat >= 0)
		insl(0x1f0, b->data, BLOCK_SIZE/4);

	b->flags |= B_VALID;	// set valid flag
	b->flags &= ~B_DIRTY;	// clear dirty flag
	ide_delay();
	wake(b);

	// Move to next item in queue
	if (idequeue != 0)
		ide_op(idequeue);

	release(&idelock);
}

void ide_init() {
	acquire(&idelock);

	pic_enable(IRQ_IDE);
	ide_wait(0);

	// Is disk 1 present?
	outb(IDE_IO | IDE_HEAD, (1<<4));

	for (int i = 0; i < 1000; i++) {
		if (inb(IDE_IO | IDE_CMD)) {
			present = 1;
			break;
		}
	}


	printf("IDE status: %d\n", present);
	//outb(IDE_IO | IDE_HEAD, 0xE0 | (0<<4));
	release(&idelock);
}


int ide_rw(buffer* b) {
	if ((b->flags & (B_VALID | B_DIRTY)) == B_VALID)
		return -1;	// only valid is set, nothing to read or write
	if (b->dev != 1)
		return -2;

	acquire(&idelock);
	buffer** pp;
	for(pp = &idequeue; *pp; pp=&(*pp)->next) 
		;	// Cycle through all valid next's.
	*pp = b;	// b is now appended.
	printf("%x, %x\n", pp, &idequeue);

	if (idequeue == b)
		ide_op(b);	// If this is only item in queue, start operation.


	while( b->flags & (B_VALID | B_DIRTY) != B_VALID)
		sleep(b, &idelock);	// Sleep until VALID is set and DIRTY is clear

	release(&idelock);
	return 0;
}

void ide_test() {
	buffer* b = malloc(sizeof(buffer));
	b->flags = 0;
	b->dev = 1;
	b->block = 7;

	buffer* c = malloc(sizeof(buffer));
	c->flags = 0;
	c->dev = 1;
	c->block = 1;
	printf("RW status: %d\n", ide_rw(b));
	printf("RW status: %d\n", ide_rw(c));

	int ticks = get_ticks();
	while(ticks + 20 > get_ticks());

	for (int i = 0; i < BLOCK_SIZE; i++)
		printf("%x", b->data[i]);
	printf("\nBlock2\n");
	for (int i = 0; i < BLOCK_SIZE; i++)
		printf("%x", c->data[i]);
}
