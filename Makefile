#baremetal makefile
#2007-2016, Michael Lazear

FINAL	= bin/kernel.bin	# Output binary
START	= start.so			# Must link this first
OBJS	= *.o				# Elf object files
AOBJS	= vectors.so trap_handler.so sched.so syscall.so int32.so font.so
INIT 	= initcode
CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm
AR		= /home/lazear/opt/cross/bin/i686-elf-as
CP		= cp
LIBGCC	= /home/lazear/opt/cross/lib/gcc/i686-elf/4.9.1/libgcc.a
CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -I ./kernel/include -c 
LDFLAGS	= -Map map.txt -T linker.ld -o $(FINAL) $(START) $(AOBJS) $(OBJS) $(LIBGCC) -b binary $(INIT)
ASFLAGS = -f elf 

all: compile link clean
build: compile link 
emu: compile link clean run

boot:
	nasm -f bin kernel/bootstrap.asm -o kernel/bootstrap
	nasm -f bin kernel/stage2.asm -o kernel/stage2
	dd if=kernel/bootstrap of=ext2 conv=notrunc
	dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc

	#cat kernel/stage2 >> kernel/bootstrap
	#cat exttrunc >> kernel/bootstrap

compile:
	#Compile C source
	$(CC) $(CCFLAGS) */*.c		# Compile top level
	$(CC) $(CCFLAGS) */*/*.c
	#$(CC) $(CCFLAGS) */*/*/*.c
	#Assembly
	$(AS) $(ASFLAGS) kernel/arch/start.s -o start.so
	$(AS) $(ASFLAGS) kernel/arch/sched.s -o sched.so
	$(AS) $(ASFLAGS) kernel/arch/syscall.s -o syscall.so
	$(AS) $(ASFLAGS) kernel/arch/trap_handler.s -o trap_handler.so
	$(AS) $(ASFLAGS) kernel/arch/vectors.s -o vectors.so
	$(AS) $(ASFLAGS) kernel/arch/int32.s -o int32.so
	$(AS) $(ASFLAGS) kernel/font.s -o font.so
	$(AS) -f bin kernel/arch/initcode.s -o initcode

hd:
	dd if=/dev/zero of=ext2 bs=1k count=16k
	sudo mke2fs ext2
	dd if=kernel/bootstrap of=ext2 conv=notrunc
	dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc

run:
	qemu-system-i386 -kernel bin/kernel.bin -hdb ext2 -curses
	
link:
	$(LD) $(LDFLAGS)	# Link using the i586-elf toolchain
	
clean:
	rm *.o				# Delete all of the object files
	rm *.so				# Delete
	
debug:
	gdb
	

	
