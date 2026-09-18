/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-allocator.h>

namespace stacsos::kernel::mem {

#define INVALID_PFN (~(pfn_t)0)

class page_allocator_buddy : public page_allocator {
public:
	page_allocator_buddy(memory_manager &mm)
		: page_allocator(mm)
	{
		for (int i = 0; i <= LAST_ORDER; i++) {
			free_list_[i] = INVALID_PFN;
		}
	}

	virtual void insert_free_pages(pfn_t range_start, u64 page_count) override;

	virtual page_allocation_result allocate_pages(order_t order, page_allocation_flags flags = page_allocation_flags::none) override;
	virtual page_allocator_error free_pages(pfn_t range_start, order_t order) override;

	virtual void dump() const override;
	virtual page_allocator_stats get_stats() const override;

private:
	static const order_t LAST_ORDER = 16;

	pfn_t free_list_[LAST_ORDER + 1];

	constexpr u64 pages_per_block(order_t order) const { return 1 << order; }
	constexpr bool block_aligned(order_t order, pfn_t pfn) { return !(pfn & (pages_per_block(order) - 1)); }

	void insert_free_block(order_t order, pfn_t block_start);
	void remove_free_block(order_t order, pfn_t block_start);

	void split_block(order_t order, pfn_t block_start);
	void merge_buddies(order_t order, pfn_t either_buddy);
};
} // namespace stacsos::kernel::mem
