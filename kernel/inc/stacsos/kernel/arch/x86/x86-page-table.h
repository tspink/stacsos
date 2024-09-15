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

class x86_page_table {
public:
	static x86_page_table *create_empty(mem::page_table_allocator &pta);

	x86_page_table *create_linked_copy(mem::page_table_allocator &pta);

	void activate()
	{
		u64 cr3val = (u64)&pml4_ - 0xffff'8000'0000'0000;
		asm volatile("mov %0, %%cr3" ::"r"(cr3val) : "memory");
	}

	void map(mem::page_table_allocator &pta, u64 virtual_address, u64 physical_address, mapping_flags flags, mapping_size size = mapping_size::m4k);
	void unmap(mem::page_table_allocator &pta, u64 virtual_address);

	void dump() const;

	u64 effective_cr3() const { return (u64)&pml4_ - 0xffff'8000'0000'0000; }

private:
	x86_page_table() = delete;
	DELETE_DEFAULT_COPY_AND_MOVE(x86_page_table)

	pml4 pml4_;
} __packed;
} // namespace stacsos::kernel::arch::x86
