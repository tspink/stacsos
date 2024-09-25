/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-alloc-ref.h>

namespace stacsos::kernel::mem {

class page;
class memory_manager;

enum class page_allocation_flags { none = 0, zero = 1 };

DEFINE_ENUM_FLAG_OPERATIONS(page_allocation_flags)

class page_allocator {
public:
	page_allocator(memory_manager &mm)
		: mm_(mm)
	{
	}

	virtual void insert_pages(page &range_start, u64 page_count) = 0;
	virtual void remove_pages(page &range_start, u64 page_count) = 0;

	virtual page *allocate_pages(int order, page_allocation_flags flags = page_allocation_flags::none) = 0;
	virtual void free_pages(page &base, int order) = 0;

	page_alloc_ref allocate_pages_ref(int order, page_allocation_flags flags = page_allocation_flags::none)
	{
		return page_alloc_ref(allocate_pages(order, flags), order);
	}

	virtual void dump() const = 0;

	void perform_selftest();

private:
	memory_manager &mm_;
};
} // namespace stacsos::kernel::mem
