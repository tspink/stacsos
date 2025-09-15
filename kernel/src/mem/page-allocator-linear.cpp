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
	page *next_free;
	u64 free_block_size;
};

static inline page_metadata *metadata(page *page) { return (page_metadata *)page->base_address_ptr(); }

void page_allocator_linear::insert_free_pages(page &range_start, u64 page_count)
{
	page **slot = &free_list_;

	while (*slot) {
		slot = &(metadata(*slot)->next_free);
	}

	*slot = &range_start;
	metadata(&range_start)->next_free = nullptr;
	metadata(&range_start)->free_block_size = page_count;
}

page *page_allocator_linear::allocate_pages(int order, page_allocation_flags flags)
{
	u64 page_count = 1 << order;

	// find a free block with enough pages
	// take from the end, so we can just reduce the free block size

	page *free_block = free_list_;

	while (free_block) {
		// TODO: Little hack here -- we subtract one so that we don't have to remove the free
		// block from the list when it's fully used up, since metadata is stored at the start
		// of the free block.
		if ((metadata(free_block)->free_block_size - 1) >= page_count) {
			metadata(free_block)->free_block_size -= page_count;

			u64 start_pfn = free_block->pfn() + metadata(free_block)->free_block_size;
			if ((flags & page_allocation_flags::zero) == page_allocation_flags::zero) {
				memops::pzero(page::get_from_pfn(start_pfn).base_address_ptr(), page_count);
			}

			return &page::get_from_pfn(start_pfn);
		}

		free_block = metadata(free_block)->next_free;
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
		u64 free_block_end = free_block_start + metadata(free_block)->free_block_size;
		dprintf("  block start=0x%lx, end=0x%lx, size=0x%x\n", free_block_start, free_block_end, free_block_end - free_block_start);

		free_block = metadata(free_block)->next_free;
	}
}
