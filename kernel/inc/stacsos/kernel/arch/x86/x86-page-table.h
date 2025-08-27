/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/page-table-structures.h>

namespace stacsos::kernel::mem {
class page_table_allocator;
}

namespace stacsos::kernel::arch::x86 {
enum class mapping_size { m4k, m2m, m1g };
enum class mapping_flags { none, present = 1, writable = 2, user_accessable = 4, write_through = 8, cache_disabled = 16 };

DEFINE_ENUM_FLAG_OPERATIONS(mapping_flags)

enum class mapping_result { ok, unmapped };

struct mapping {
	mapping_result result;
	u64 address;
};

class x86_page_table {
public:
	/**
	 * @brief Creates a new, empty, X86 page table, and returns a pointer to the newly created object.
	 *
	 * @param pta The allocator to use for allocating page tables.
	 * @return x86_page_table* The newly created page table.
	 */
	static x86_page_table *create_empty(mem::page_table_allocator &pta);

	/**
	 * @brief Creates a new x86 page table, with all top-level (i.e. pml4) entries as copies of the
	 * existing page table's pml4 entries.
	 *
	 * @param pta The allocator to use for allocating page tables.
	 * @return x86_page_table* THe newly created page table.
	 */
	x86_page_table *create_linked_copy(mem::page_table_allocator &pta);

	/**
	 * @brief Retrieves a pointer to the current X86 page table.
	 *
	 * @return x86_page_table*
	 */
	static x86_page_table *current()
	{
		u64 cr3val;
		asm volatile("mov %%cr3, %0" : "=r"(cr3val));

		// Mask out CR3 flags.
		cr3val &= ~0xfffull;

		// This is safe because the cr3 address is a physical address, so we can
		// easily turn it into a virtual address.
		return (x86_page_table *)(0xffff'8000'0000'0000 + cr3val);
	}

	void activate()
	{
		// This should be safe to do, because the PML4 should have
		// been allocated by the page table allocator, and we should
		// be operating via a physical address in the 1-1 mapping area.

		u64 pml4_address = (u64)&pml4_;

		// TODO: Validate that PML4 address is within the physical mapping extents

		u64 cr3val = pml4_address - 0xffff'8000'0000'0000;
		asm volatile("mov %0, %%cr3" ::"r"(cr3val) : "memory");
	}

	/**
	 * @brief Adds a new mapping into the page table, mapping a virtual address to a physical address, with the requested flags.
	 *
	 * @param pta The allocator to use for allocating page tables.
	 * @param virtual_address The virtual address to be mapped.
	 * @param physical_address The physical address the virtual address is being mapped to.
	 * @param flags The flags (i.e. permissions, etc) to use for the mapping.
	 * @param size The granularity of the mapping.
	 */
	void map(mem::page_table_allocator &pta, u64 virtual_address, u64 physical_address, mapping_flags flags, mapping_size size = mapping_size::m4k);

	/**
	 * @brief Removes the mapping for a virtual address from the page table.
	 *
	 * @param pta The allocator to use for allocating page tables.
	 * @param virtual_address The virtual address to remove from the mapping.
	 */
	void unmap(mem::page_table_allocator &pta, u64 virtual_address);

	/**
	 * @brief Looks up an existing mapping (if it exists) and returns details about it.
	 *
	 * @param virtual_address The virtual address to look up the mapping for.  This does NOT need to be page aligned.
	 * @return mapping A mapping object, containing either an error, or the resulting address of the mapping if it exists.
	 */
	mapping get_mapping(u64 virtual_address);

	void dump() const;

	u64 effective_cr3() const { return (u64)&pml4_ - 0xffff'8000'0000'0000; }

private:
	// Delete the default constructor and destructor.
	x86_page_table() = delete;
	~x86_page_table() = delete;

	DELETE_DEFAULT_COPY_AND_MOVE(x86_page_table)

	// The top-level X86 page table, i.e. the PML4.  This needs to be the one and only field in this class, as we need to be able to
	// re-interpret pointers.
	pml4 pml4_;
} __packed;

static_assert(sizeof(x86_page_table) == sizeof(pml4), "X86 Page Table convenience class has incorrect size");
} // namespace stacsos::kernel::arch::x86
