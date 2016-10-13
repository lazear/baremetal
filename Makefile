#xiphos makefile
#2007-2016, Michael Lazear

PRE 	= cross/bin
FINAL	= bin/kernel.bin	# Output binary
START	= start.so			# Must link this first
OBJS	= *.o				# Elf object files
AOBJS	= vectors.so trap_handler.so sched.so syscall.so font.so umode.so
INIT 	= initcode
CC	= ~/opt/cross/bin/i686-elf-gcc
LD	= ~/opt/cross/bin/i686-elf-ld
AS	= nasm
AR	= ~/opt/cross/bin/i686-elf-as
CP	= cp
LIBGCC	= ~/opt/cross/lib/gcc/i686-elf/7.0.0/libgcc.a
CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -I kernel/include/  -c 
LDFLAGS	= -Map map.txt -T linker.ld -o $(FINAL) $(START) $(AOBJS) $(OBJS) $(LIBGCC) -b binary $(INIT) ap_entry
ASFLAGS = -f elf 
EXTUTIL = ../ext2util/ext2util
IMAGE	= ext2

all: compile link clean
user: userland emu
build: compile link 
emu: compile link clean run


userland:
	$(CC) -print-sysroot
	make -C libc
	$(CC) -w --sysroot=libc -I libc/usr/include user/user.c -o user.elf
	$(CC) -w --sysroot=libc -I libc/usr/include user/lass.c -o lass.elf
	$(CC) -w --sysroot=libc lquad.c -o lquad.elf

	$(EXTUTIL) -x $(IMAGE) -wf user.elf
	$(EXTUTIL) -x $(IMAGE) -wf lass.elf
	$(EXTUTIL) -x $(IMAGE) -wf lquad.elf
	objdump -d user.elf > user.S

lass:
	$(CC) -w --sysroot=libc user/lass.c -o lass.elf

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
	$(AS) $(ASFLAGS) kernel/arch/umode.s -o umode.so
	#$(CC) -c kernel/arch/umode.s -o umode.so
	$(AS) -f bin kernel/arch/initcode.s -o initcode
	$(AS) -f bin kernel/arch/ap_entry.s -o ap_entry
	# $(LD) -N -e start -Ttext 0x8000 -o ap_entry.o ap_entry.elf
	# objcopy -S -O binary -j .text ap_entry.o ap_entry

hd:
	dd if=/dev/zero of=ext2 bs=1k count=16k
	sudo mke2fs ext2
#	dd if=kernel/bootstrap of=ext2 conv=notrunc
#	dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc
	cp bin/kernel.bin .
	$(EXTUTIL) -x $(IMAGE) -wf kernel.bin
	rm kernel.bin
run:
	./qemu.sh
link:
	$(LD) $(LDFLAGS)	# Link using the i586-elf toolchain
	
clean:
	rm *.o				# Delete all of the object files
	rm *.so				# Delete
	
debug:
	gdb
	

	
