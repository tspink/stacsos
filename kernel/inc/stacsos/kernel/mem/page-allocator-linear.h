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
		, free_list_start_(0)
	{
	}

	virtual void insert_free_pages(pfn_t range_start, u64 page_count) override;

	virtual page_allocation_result allocate_pages(order_t order, page_allocation_flags flags = page_allocation_flags::none) override;
	virtual page_allocator_error free_pages(pfn_t range_start, order_t order) override;

	virtual void dump() const override;

	virtual page_allocator_stats get_stats() const override { return page_allocator_stats { total_pages_ - free_pages_, free_pages_ }; }

private:
	pfn_t free_list_start_;
	u64 total_pages_;
	u64 free_pages_;
};
} // namespace stacsos::kernel::mem
