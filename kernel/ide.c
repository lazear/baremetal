/*
ide.c

Disk buffer cache/LBA read/write support
*/

#include <types.h>
#include <x86.h>
#include <mutex.h>
#include <traps.h>

#define BLOCK_SIZE		512
#define SECTOR_SIZE		512

#define B_BUSY	0x1		// buffer is locked by a process
#define B_VALID	0x2		// buffer has been read from disk
#define B_DIRTY	0x4		// buffer has been written to


/* Define IDE status bits */
#define IDE_BSY 		(1<<7)	// Drive is preparing to send/receive data
#define IDE_RDY 		(1<<6)	// Clear when drive is spun down, or after error
#define IDE_DF			(1<<5)	// Drive Fault error
#define IDE_ERR			(1<<0)	// Error has occured

#define IDE_IO			0x1F0	// Main IO port
#define IDE_DATA		0x0 	// R/W PIO data bytes
#define IDE_FEAT		0x1 	// ATAPI devices
#define IDE_SECN		0x2 	// # of sectors to R/W
#define IDE_LOW			0x3 	// CHS/LBA28/LBA48 specific
#define IDE_MID 		0x4
#define IDE_HIGH		0x5
#define IDE_HEAD		0x6 	// Select drive/heaad
#define IDE_CMD 		0x7 	// Command/status port
#define IDE_ALT			0x3F6	// alternate status
#define LBA_LOW(c)		((uint8_t) (c & 0xFF))
#define LBA_MID(c)		((uint8_t) (c >> 8) & 0xFF)
#define LBA_HIGH(c)		((uint8_t) (c >> 16) & 0xFF)
#define LBA_LAST(c)		((uint8_t) (c >> 24) & 0xF)

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30


#define MAX_OP_BLOCKS	16	// Max # of blocks any operation can write (8 KB)


void sleep(void* channel, mutex* m) {
	release(m);
}
void wake(void* channel) {} 

mutex idelock = {.lock = 0};

typedef struct ide_buffer {
	uint8_t flags;				// buffer flags
	uint32_t dev;				// device number
	uint32_t block;				// block number
	struct ide_buffer* next;	// next block in queue
	uint8_t data[BLOCK_SIZE];	// 1 disk sector of data
} buffer;

// This is a pointer to the buffer being currently processed
buffer* idequeue;

static int present = 0;

void buffer_init() {
	idequeue = malloc(sizeof(buffer) * MAX_OP_BLOCKS);	
	for (buffer* q = idequeue; q < idequeue + MAX_OP_BLOCKS; q++) {
		memset(q, 0, sizeof(buffer));
		q->next = idequeue;
	}
}



/*
Wait for IDE device to become ready
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
	outb(0x3F6, 0);	// Generate interrupt
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
	printf("RW status: %d\n", ide_rw(b));

	int ticks = get_ticks();
	while(ticks + 10 > get_ticks());

	for (int i = 0; i < BLOCK_SIZE; i++)
		printf("%x", b->data[i]);
}
