/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
template <int bits> struct elf_header;

enum class elf_ident_classes : u8 {
	ei_class_32bit = 1,
	ei_class_64bit = 2,
};

enum class elf_ident_data : u8 {
	ei_data_le = 1,
	ei_data_be = 2,
};

enum class elf_ident_osabi : u8 {
	ei_osabi_system_v = 0,
	ei_osabi_hp_ux = 1,
	ei_osabi_netbsd = 2,
	ei_osabi_linux = 3,
};

enum class elf_header_type : u16 {
	e_type_none = 0,
	e_type_rel = 1,
	e_type_exec = 2,
	e_type_dyn = 3,
};

enum class elf_header_machine : u16 {
	e_machine_none = 0,
	e_machine_x86_32 = 3,
	e_machine_x86_64 = 0x3e,
};

enum class elf_program_header_type : u32 { pt_null = 0, pt_load = 1, pt_dynamic = 2 };
enum class elf_program_header_flags : u32 { pf_x = 1, pf_w = 2, pf_r = 4 };

struct elf_ident_header {
	u8 ei_magic[4];
	elf_ident_classes ei_class;
	elf_ident_data ei_data;
	u8 ei_version;
	elf_ident_osabi ei_osabi;
	u8 ei_abiversion;
	u8 ei_pad[7];
} __packed;

static_assert(sizeof(elf_ident_header) == 16, "Incorrect size for ELF ident header");

template <> struct elf_header<64> {
	elf_ident_header e_ident;
	elf_header_type e_type;
	elf_header_machine e_machine;
	u32 e_version;
	u64 e_entry;
	u64 e_phoff;
	u64 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} __packed;

template <> struct elf_header<32> {
	struct {
		u8 ei_magic[4];
		u8 ei_class;
		u8 ei_data;
		u8 ei_version;
		u8 ei_osabi;
		u8 ei_abiversion;
		u8 ei_pad[7];
	} __packed e_ident;

	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u32 e_entry;
	u32 e_phoff;
	u32 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} __packed;

static_assert(sizeof(elf_header<32>) == 0x34, "incorrect elf_header-32 size");
static_assert(sizeof(elf_header<64>) == 0x40, "incorrect elf_header-64 size");

template <int bits> struct elf_sectionheader;

template <> struct elf_sectionheader<64> {
	u32 sh_name;
	u32 sh_type;
	u64 sh_flags;
	u64 sh_addr;
	u64 sh_offset;
	u64 sh_size;
	u32 sh_link;
	u32 sh_info;
	u64 sh_addralign;
	u64 sh_entsize;
} __packed;

template <> struct elf_sectionheader<32> {
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} __packed;

static_assert(sizeof(elf_sectionheader<32>) == 0x28, "incorrect elf_sectionheader-32 size");
static_assert(sizeof(elf_sectionheader<64>) == 0x40, "incorrect elf_sectionheader-64 size");

#define SHT_NULL 0
#define SHT_SYMTAB 2

template <int bits> struct elf_sym;

template <> struct elf_sym<64> {
	u32 st_name;
	u8 st_info;
	u8 st_other;
	u16 st_shndx;
	u64 st_value;
	u64 st_size;
};

template <> struct elf_sym<32> {
	u32 st_name;
	u32 st_value;
	u32 st_stize;
	u8 st_info;
	u8 st_other;
	u16 st_shndx;
};

template <int bits> struct elf_programheader;

template <> struct elf_programheader<64> {
	elf_program_header_type p_type;
	elf_program_header_flags p_flags;
	u64 p_offset;
	u64 p_vaddr;
	u64 p_paddr;
	u64 p_filesz;
	u64 p_memsz;
	u64 p_align;
};

// static_assert(sizeof(elf_programheader<32>) == 0x28, "incorrect elf_programheader-32 size");
static_assert(sizeof(elf_programheader<64>) == 0x38, "incorrect elf_programheader-64 size");

} // namespace stacsos
