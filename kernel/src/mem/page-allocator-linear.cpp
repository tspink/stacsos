/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/page-allocator-linear.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel::mem;

struct page_metadata {
	pfn_t next_free;
	u64 free_block_size;
};

static inline page_metadata *metadata(pfn_t pfn) { return (page_metadata *)page::get_from_pfn(pfn).base_address_ptr(); }

void page_allocator_linear::insert_free_pages(pfn_t range_start, u64 page_count)
{
	if (free_list_start_ == 0) {
		free_list_start_ = range_start;
		metadata(free_list_start_)->next_free = 0;
		metadata(free_list_start_)->free_block_size = page_count;
	} else {
		pfn_t current_free_block = free_list_start_;

		do {
			pfn_t next_free_block = metadata(current_free_block)->next_free;
			if (next_free_block == 0) {
				break;
			}

			current_free_block = next_free_block;
		} while (1);

		metadata(current_free_block)->next_free = range_start;
		metadata(range_start)->next_free = 0;
		metadata(range_start)->free_block_size = page_count;
	}

	total_pages_ += page_count;
	free_pages_ += page_count;
}

page_allocation_result page_allocator_linear::allocate_pages(order_t order, page_allocation_flags flags)
{
	u64 page_count = 1 << order;

	// find a free block with enough pages
	// take from the end, so we can just reduce the free block size

	pfn_t free_block = free_list_start_;

	while (free_block) {
		// TODO: Little hack here -- we subtract one so that we don't have to remove the free
		// block from the list when it's fully used up, since metadata is stored at the start
		// of the free block.
		if ((metadata(free_block)->free_block_size - 1) >= page_count) {
			metadata(free_block)->free_block_size -= page_count;

			u64 start_pfn = free_block + metadata(free_block)->free_block_size;
			if ((flags & page_allocation_flags::zero) == page_allocation_flags::zero) {
				memops::pzero(page::get_from_pfn(start_pfn).base_address_ptr(), page_count);
			}

			return page_allocation_result::ok(start_pfn);
		}

		free_block = metadata(free_block)->next_free;
	}

	return page_allocation_result::error(page_allocator_error::out_of_memory);
}

page_allocator_error page_allocator_linear::free_pages(pfn_t range_start, order_t order) { return page_allocator_error::not_implemented; }

void page_allocator_linear::dump() const
{
	pfn_t free_block = free_list_start_;
	if (free_block == 0) {
		dprintf("[empty]\n");
		return;
	}

	dprintf("free list:\n");
	while (free_block) {
		u64 free_block_start = free_block;
		u64 free_block_end = free_block_start + metadata(free_block)->free_block_size;
		dprintf("  block start=0x%lx, end=0x%lx, size=0x%x\n", free_block_start, free_block_end, free_block_end - free_block_start);

		free_block = metadata(free_block)->next_free;
	}
}
