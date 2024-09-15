ENTRY(multiboot_start)

PHDRS
{
	headers PT_PHDR PHDRS;
	startup_header PT_LOAD FILEHDR PHDRS;
	startup_text PT_LOAD;
	startup_data PT_LOAD;
	text PT_LOAD;
	data PT_LOAD;
}

SECTIONS
{
	. = 4M;

	_IMAGE_START = .;

	. += SIZEOF_HEADERS;

	/* Multiboot & Startup Code */

	_STARTUP_START = .;
	.startup.header : AT(_STARTUP_START) { *(.multiboot.header) } :startup_header

	. = ALIGN(16);

	.startup.text :
	{
		*(.multiboot.text)
		*(.startup.text)
		*(.startup.text32)
		*(.startup.text64)
	} :startup_text

	. = ALIGN(16);

	.startup.data :
	{
		*(.startup.data)
		*(.startup.rodata)
	} :startup_data

	_STARTUP_END = .;

	/* Main Kernel Code and Data */

	/* text */

	. = ALIGN(4096);
	_KERNEL_LMA_START = .;

    . = 0xFFFFFFFF80000000 + _KERNEL_LMA_START;
    _KERNEL_VMA_START = .;

	_TEXT_START = .;
	.text : AT(_KERNEL_LMA_START)
	{
		*(.text)
		*(.text.*)
	} :text
	_TEXT_END = .;

	/* rodata */

	. = ALIGN(4096);

	_RODATA_START = .;
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		*(.ehframe)
	}
	_RODATA_END = .;

	/* data */

	. = ALIGN(4096);

	_DATA_START = .;
	.data :
	{
		*(.data)

		. = ALIGN(16);

		__init_array_start = .;
		KEEP(*(.init_array*))
		__init_array_end = .;
	} :data
	_DATA_END = .;

	/* bss */

	. = ALIGN(4096);

	_BSS_START = .;
	.bss : {
		*(.bss)
		*(.bss.*)
	}
	_BSS_END = .;

	/* end of image */

	_IMAGE_END = . - 0xFFFFFFFF80000000;

	. = ALIGN(4096);

	_DYNAMIC_DATA_START = .;

	/DISCARD/ :
	{
		*(.comment)
		*(.note)
		*(.note.*)
	}
}
