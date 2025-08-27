/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-allocator.h>

namespace stacsos::kernel::mem {
class page_allocator_buddy : public page_allocator {
public:
	page_allocator_buddy(memory_manager &mm)
		: page_allocator(mm)
	{
		for (int i = 0; i <= LastOrder; i++) {
			free_list_[i] = nullptr;
		}
	}

	virtual void insert_free_pages(page &range_start, u64 page_count) override;

	virtual page *allocate_pages(int order, page_allocation_flags flags = page_allocation_flags::none) override;
	virtual void free_pages(page &base, int order) override;

	virtual void dump() const override;

private:
	static const int LastOrder = 16;

	page *free_list_[LastOrder + 1];
	u64 total_free_;

	constexpr u64 pages_per_block(int order) const { return 1 << order; }

	constexpr bool block_aligned(int order, u64 pfn) { return !(pfn & (pages_per_block(order) - 1)); }

	void insert_free_block(int order, page &block_start);
	void remove_free_block(int order, page &block_start);

	void split_block(int order, page &block_start);
	void merge_buddies(int order, page &buddy);
};
} // namespace stacsos::kernel::mem
