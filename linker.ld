OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(entry)
SECTIONS
{
	. = 0xC0100000;
	start = .;
	.text :	AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
		. = ALIGN(4096);
	}

	.data :AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}
	
	.bss :AT(ADDR(.bss) - 0xC0000000)
	{
		*(.bss)
		*(stack_bottom)
		*(stack_top)
		. = ALIGN(4096);
	}
	end = .;
	kernel_end = .;
}