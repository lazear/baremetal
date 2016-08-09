#baremetal makefile
#2007-2016, Michael Lazear

FINAL	= bin/kernel.bin	# Output binary
START	= start.so			# Must link this first
OBJS	= *.o				# Elf object files
CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm
AR		= /home/lazear/opt/cross/bin/i686-elf-as
CP		= cp

CCFLAGS	= -O -w -fno-builtin -nostdlib -ffreestanding -m32 -I ./include -c 
LDFLAGS	= -Map map.txt -T linker.ld -o $(FINAL) $(START) $(OBJS)
ASFLAGS = -f elf 

all: compile link clean
build: compile link 


compile: *.c
	#Compile C source
	$(CC) $(CCFLAGS) *.c		# Compile top level

	#Assembly
	$(AS) $(ASFLAGS) start.s -o start.so
	
link:
	$(LD) $(LDFLAGS)	# Link using the i586-elf toolchain
	
clean:
	rm *.o				# Delete all of the object files
	rm *.so				# Delete
	

	

	
