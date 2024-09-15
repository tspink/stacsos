/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86::boot {
struct multiboot_elf_section_header_table {
	u32 num;
	u32 size;
	u32 addr;
	u32 shndx;
} __packed;

enum class multiboot_mmap_entry_type : u32 {
	MMAP_ENTRY_TYPE_AVAILABLE = 1,
	MMAP_ENTRY_TYPE_RESERVED = 2,
};

struct multiboot_mmap_entry {
	u32 size;
	u64 addr;
	u64 len;
	multiboot_mmap_entry_type type;
} __packed;

struct multiboot_module_entry {
	/* the memory used goes from bytes 'mod_start' to
	 * 'mod_end-1' inclusive */
	u32 mod_start;
	u32 mod_end;

	/* Module command line */
	u32 cmdline;

	/* padding to take it to 16 bytes (must be zero) */
	u32 pad;
};

struct multiboot_info {
	/* Multiboot info version number */
	u32 flags;

	/* Available memory from BIOS */
	u32 mem_lower;
	u32 mem_upper;

	/* "root" partition */
	u32 boot_device;

	/* Kernel command line */
	u32 cmdline;

	/* Boot-Module list */
	u32 mods_count;
	u32 mods_addr;

	struct multiboot_elf_section_header_table elf_sec;

	/* Memory Mapping buffer */
	u32 mmap_length;
	u32 mmap_addr;

	/* Drive Info buffer */
	u32 drives_length;
	u32 drives_addr;

	/* ROM configuration table */
	u32 config_table;

	/* Boot Loader Name */
	u32 boot_loader_name;

	/* APM table */
	u32 apm_table;

	/* Video */
	u32 vbe_control_info;
	u32 vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_interface_seg;
	u16 vbe_interface_off;
	u16 vbe_interface_len;
};
} // namespace stacsos::kernel::arch::x86::boot
