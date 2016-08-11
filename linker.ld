OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(entry)
SECTIONS
{
	. = 0x00100000;
	start = .;
	.text :	
	{
		*(.text)
		. = ALIGN(4096);
	}

	.data :
	{
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}
	
	.bss :
	{
		*(.bss)
		. = ALIGN(4096);
	}
	end = .;
	kernel_end = .;
}