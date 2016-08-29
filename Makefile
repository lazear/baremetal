#baremetal makefile
#2007-2016, Michael Lazear

FINAL	= bin/kernel.bin	# Output binary
START	= start.so			# Must link this first
OBJS	= *.o				# Elf object files
AOBJS	= vectors.so trap_handler.so sched.so syscall.so int32.so
INIT 	= initcode
CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm
AR		= /home/lazear/opt/cross/bin/i686-elf-as
CP		= cp

CCFLAGS	= -O -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -I ./kernel/include -c 
LDFLAGS	= -Map map.txt -T linker.ld -o $(FINAL) $(START) $(AOBJS) $(OBJS) -b binary $(INIT)
ASFLAGS = -f elf 

all: compile link clean
build: compile link 
db: compile link clean debug

boot:
	nasm -f bin kernel/bootstrap.asm -o kernel/bootstrap
	nasm -f bin kernel/stage2.asm -o kernel/stage2
	cat kernel/stage2 >> kernel/bootstrap

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

	$(AS) -f bin kernel/arch/initcode.s -o initcode



	
link:
	$(LD) $(LDFLAGS)	# Link using the i586-elf toolchain
	
clean:
	rm *.o				# Delete all of the object files
	rm *.so				# Delete
	
debug:
	gdb
	

	
