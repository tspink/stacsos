/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/page-allocator-linear.h>

using namespace stacsos::kernel::mem;

void page_allocator_linear::insert_pages(page &range_start, u64 page_count)
{
	page **slot = &free_list_;

	while (*slot) {
		slot = &(*slot)->next_free_;
	}

	*slot = &range_start;
	range_start.free_block_size_ = page_count;
}

void page_allocator_linear::remove_pages(page &range_start_r, u64 page_count)
{
	page *free_block = free_list_;

	while (free_block) {
		u64 free_block_start = free_block->pfn();
		u64 free_block_end = free_block_start + free_block->free_block_size_;
		u64 range_start = range_start_r.pfn();
		u64 range_end = range_start + page_count;

		// dprintf("considering %lx -- %lx for %lx -- %lx\n", free_block_start, free_block_end, range_start, range_end);

		if (range_start_r.pfn() >= free_block_start && range_start_r.pfn() < free_block_end) {
			// dprintf("  range found\n");

			u64 offset = range_start - free_block_start;
			// dprintf("  offset=%lx\n", offset);

			free_block->free_block_size_ = offset;

			u64 remainder_pages = range_end > free_block_end ? 0 : free_block_end - range_end;
			// dprintf("  remainder=%lx\n", remainder_pages);

			if (remainder_pages) {
				page *tmp_next_free = free_block->next_free_;

				free_block->next_free_ = &page::get_from_pfn(range_end);
				free_block->next_free_->free_block_size_ = remainder_pages;
				free_block->next_free_->next_free_ = tmp_next_free;
			}

			break;
		}

		free_block = free_block->next_free_;
	}
}

page *page_allocator_linear::allocate_pages(int order, page_allocation_flags flags)
{
	u64 page_count = 1 << order;

	// find a free block with enough pages
	// take from the end, so we can just reduce the free block size

	page *free_block = free_list_;

	while (free_block) {
		if (free_block->free_block_size_ >= page_count) {
			free_block->free_block_size_ -= page_count;

			u64 start_pfn = free_block->pfn() + free_block->free_block_size_;
			return &page::get_from_pfn(start_pfn);
		}

		free_block = free_block->next_free_;
	}

	return nullptr;
}

void page_allocator_linear::free_pages(page &base, int order)
{
	// TODO
}

void page_allocator_linear::dump() const
{
	page *free_block = free_list_;

	if (!free_block) {
		dprintf("[empty]\n");
		return;
	}

	dprintf("free list:\n");

	while (free_block) {
		u64 free_block_start = free_block->pfn();
		u64 free_block_end = free_block_start + free_block->free_block_size_;
		dprintf("  block start=0x%lx, end=0x%lx, size=0x%x\n", free_block_start, free_block_end, free_block_end - free_block_start);

		free_block = free_block->next_free_;
	}
}
