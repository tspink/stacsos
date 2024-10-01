/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel - Core
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/boot/multiboot.h>
#include <stacsos/kernel/arch/x86/cpuid.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-table.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::arch::x86::boot;
using namespace stacsos::kernel::mem;

// References to the C++ static constructors, and the BSS section.
extern "C" {
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

extern char _BSS_START[], _BSS_END;
}

/**
 * Zeros the BSS section.
 */
static void zero_bss()
{
	size_t length = (uintptr_t)&_BSS_END - (uintptr_t)&_BSS_START;
	memops::bzero(&_BSS_START, length);
}

/**
 * Runs the static constructors.
 */
static void run_static_constructors()
{
	size_t ctor_count = __init_array_end - __init_array_start;

	// Loop over each constructor, and call it.
	for (unsigned int i = 0; i < ctor_count; i++) {
		(*__init_array_start[i])();
	}
}

/**
 * Initialises memory, by scanning the memory blocks that were provided to us by the multiboot loader.
 */
static void initialise_memory(const multiboot_info *mbi)
{
	void *mmap_start = phys_to_virt(mbi->mmap_addr);
	void *mmap_end = (void *)((uintptr_t)mmap_start + mbi->mmap_length);

	// Loop over each MMAP entry, and tell the memory manager of its existence.
	multiboot_mmap_entry *mmap = (multiboot_mmap_entry *)mmap_start;
	while ((uintptr_t)mmap < (uintptr_t)mmap_end) {
		memory_manager::add_memory_block(mmap->addr, mmap->len, mmap->type == multiboot_mmap_entry_type::MMAP_ENTRY_TYPE_AVAILABLE);

		mmap = (multiboot_mmap_entry *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
	}
}

/* Command-line Handling */
static char __boot_command_line[256];

static void process_command_line(const multiboot_info *mbi)
{
	memops::strncpy(__boot_command_line, (const char *)phys_to_virt(mbi->cmdline), sizeof(__boot_command_line) - 1);
}

/**
 * Clears the lower one-to-one physical mapping from the page table,
 * to catch any bugs that might arise by using lower addresses.  This is
 * operating on the temporary page tables that were built during startup.
 */
static void clear_lower_mappings()
{
	u64 *cr3;

	// Read the current PML4 pointer.
	asm volatile("mov %%cr3, %0" : "=r"(cr3));

	// Delete the PDP from the lower mapping.
	cr3[0] = 0;

	// Reload the PML4, initiating a TLB flush.
	asm volatile("mov %0, %%cr3" ::"r"(cr3));
}

static void check_arch_support()
{
	cpuid c;
	c.initialise();

#ifdef USE_FSGSBASE
	if (!c.get_feature(cpuid_features::fsgsbase)) {
		dprintf("\e4ERROR:\e7 FSGSBASE IS NOT SUPPORTED.\n");
		abort();
	}
#endif
}

/* Architecture-indepentent kernel entry point */
extern __noreturn void main(const char *cmdline);

/**
 * Main entry point, after coming in from the long-mode assembly code.
 * @param multiboot_info A pointer to the multiboot information structure provided at boot time by the boot loader.
 */
extern "C" __noreturn void x86_start(const multiboot_info *multiboot_info)
{
	// Clear lower mappings in the virtual address space.  This will significantly
	// help with any memory-related bugs.  Especially since it removes the zero page!
	clear_lower_mappings();

	// Clear the BSS section.
	zero_bss();

	// Run C++ static constructors.
	run_static_constructors();

	// Now we can do things!
	dprintf_init();

	dprintf("\ef\xc9\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbb\n");
	dprintf("\xba \e7Welcome to \ecStACSOS\e7! \ef\xba\n");
	dprintf("\ef\xc8\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\e7\n");

	// Make sure arch features are available.
	check_arch_support();

	// Process the command-line.
	process_command_line(multiboot_info);
	dprintf("command-line: %s\n", __boot_command_line);

	// Initialise memory.
	initialise_memory(multiboot_info);

	// Call the main kernel.
	main(__boot_command_line);

	// This is unreachable because main() does not return.
	__unreachable();
}
