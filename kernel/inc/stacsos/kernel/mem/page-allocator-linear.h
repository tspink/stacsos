/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-allocator.h>

namespace stacsos::kernel::mem {

class page_allocator_linear : public page_allocator {
public:
	page_allocator_linear(memory_manager &mm)
		: page_allocator(mm)
		, free_list_(nullptr)
	{
	}

	virtual void insert_free_pages(page &range_start, u64 page_count) override;

	virtual page *allocate_pages(int order, page_allocation_flags flags = page_allocation_flags::none) override;
	virtual void free_pages(page &base, int order) override;

	virtual void dump() const override;

private:
	page *free_list_;
};
} // namespace stacsos::kernel::mem
