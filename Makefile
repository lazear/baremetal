#crunchy makefile
#2007-2016, Michael Lazear

FINAL	= bin/kernel.bin	# Output binary
START	= start.so			# Must link this first
OBJS	= *.o				# Elf object files
AOBJS	= vectors.so trap_handler.so sched.so syscall.so font.so
INIT 	= initcode
CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm
AR		= /home/lazear/opt/cross/bin/i686-elf-as
CP		= cp
LIBGCC	= #/home/lazear/opt/cross/lib/gcc/i686-elf/7.0.0/libgcc.a
CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -I kernel/include/  -c 
LDFLAGS	= -Map map.txt -T linker.ld -o $(FINAL) $(START) $(AOBJS) $(OBJS) $(LIBGCC) -b binary $(INIT) ap_entry
ASFLAGS = -f elf 
EXTUTIL = ../ext2util/ext2util
IMAGE	= ext2

all: compile link clean
build: compile link 
emu: compile link clean run
user: userland emu

userland:
	make -C libc
	$(CC) -w -fno-builtin -ffreestanding user/user.c -o user.elf
	$(EXTUTIL) -x $(IMAGE) -wf user.elf -i 12
	objdump -d user.elf > user.S

boot:
	nasm -f bin kernel/bootstrap.asm -o kernel/bootstrap
	nasm -f bin kernel/stage2.asm -o kernel/stage2
	dd if=kernel/bootstrap of=ext2 conv=notrunc
	dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc

	#cat kernel/stage2 >> kernel/bootstrap
	#cat exttrunc >> kernel/bootstrap

compile:
	#Compile C source
	$(CC) $(CCFLAGS) kernel/*.c		# Compile top level
	$(CC) $(CCFLAGS) kernel/*/*.c
	#$(CC) $(CCFLAGS) */*/*/*.c
	#Assembly
	$(AS) $(ASFLAGS) kernel/arch/start.s -o start.so
	$(AS) $(ASFLAGS) kernel/arch/sched.s -o sched.so
	$(AS) $(ASFLAGS) kernel/arch/syscall.s -o syscall.so
	$(AS) $(ASFLAGS) kernel/arch/trap_handler.s -o trap_handler.so
	$(AS) $(ASFLAGS) kernel/arch/vectors.s -o vectors.so
	$(AS) $(ASFLAGS) kernel/font.s -o font.so
	$(AS) -f bin kernel/arch/initcode.s -o initcode
	$(AS) -f bin kernel/arch/ap_entry.s -o ap_entry
	#$(LD) -N -e start -Ttext 0x8000 -o ap_entry.o ap_entry.elf
	#objcopy -S -O binary -j .text ap_entry.o ap_entry

hd:
	dd if=/dev/zero of=ext2 bs=1k count=16k
	sudo mke2fs ext2
#	dd if=kernel/bootstrap of=ext2 conv=notrunc
#	dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc

run:
	qemu-system-x86_64 -kernel bin/kernel.bin -hdb ext2 -curses -smp cpus=2
	
link:
	$(LD) $(LDFLAGS)	# Link using the i586-elf toolchain
	
clean:
	rm *.o				# Delete all of the object files
	rm *.so				# Delete
	
debug:
	gdb
	

	
