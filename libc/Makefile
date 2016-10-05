#crunchy makefile
#2007-2016, Michael Lazear

FINAL	= new
OBJS	= *.o				# Elf object files
PREFIX  = ..# ~/opt
CC	    = $(PREFIX)/cross/bin/i686-elf-gcc
LD		= $(PREFIX)/cross/bin/i686-elf-ld
NASM		= nasm
AS 		= $(PREFIX)/cross/bin/i686-elf-as
AR		= ~/opt/cross/bin/i686-elf-ar
CP		= cp
CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 --sysroot=. -c 
ASFLAGS = -f elf

all: compile clean

compile:
	#Compile C source
	$(CC) $(CCFLAGS) usr/string/*.c	
	$(CC) $(CCFLAGS) usr/stdlib/*.c
	$(CC) $(CCFLAGS) usr/stdio/*.c
	$(CC) $(CCFLAGS) usr/*.c

	#Generate archive
	$(AR) -cvru libx.a *.o

	$(NASM) $(ASFLAGS) crt0.s
	$(AS) crti.s -o crti.o
	$(AS) crtn.s -o crtn.o

	cp libx.a usr/lib/
	cp crt*.o usr/lib/
	cp usr/lib/libx.a usr/lib/libc.a

	
clean:
	rm *.o				# Delete all of the object files
	rm *.a
	
	

	
