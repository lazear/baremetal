#crunchy makefile
#2007-2016, Michael Lazear

FINAL	= new
OBJS	= *.o				# Elf object files
CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm
AR		= /home/lazear/opt/cross/bin/i686-elf-ar
CP		= cp
CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -isystem=usr/include --sysroot=. -I ./usr/include/ -c 
ASFLAGS = -f elf

compile:
	#Compile C source
	$(CC) $(CCFLAGS) *.c		# Compile top level
	$(AS) $(ASFLAGS) crt0.s
	$(AS) $(ASFLAGS) crti.s
	$(AR) -cvq libcrunch.a s*.o

link:
	$(LD) -o $(FINAL) crt0.o crti.o new.o libcrunch.a
	
clean:
	rm *.o				# Delete all of the object files
	
	

	
