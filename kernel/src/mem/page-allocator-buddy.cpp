/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/page-allocator-buddy.h>
#include <stacsos/kernel/mem/page.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::mem;

void page_allocator_buddy::dump() const
{
	dprintf("*** buddy page allocator - free list ***\n");

	for (int i = 0; i <= LastOrder; i++) {
		dprintf("[%02u] ", i);

		page *c = free_list_[i];
		while (c) {
			dprintf("%lx--%lx ", c->base_address(), (c->base_address() + ((1 << i) << PAGE_BITS)) - 1);
			c = c->next_free_;
		}

		dprintf("\n");
	}
}

void page_allocator_buddy::insert_pages(page &range_start, u64 page_count) { panic("TODO"); }

void page_allocator_buddy::remove_pages(page &range_start, u64 page_count) { panic("TODO"); }

void page_allocator_buddy::insert_free_block(int order, page &block_start)
{
	// assert order in range
	assert(order >= 0 && order <= LastOrder);

	// assert block_start aligned to order
	assert(block_aligned(order, block_start.pfn()));

	page *target = &block_start;
	page **slot = &free_list_[order];
	while (*slot && *slot < target) {
		slot = &((*slot)->next_free_);
	}

	assert(*slot != target);

	target->next_free_ = *slot;
	*slot = target;
}

void page_allocator_buddy::remove_free_block(int order, page &block_start)
{
	// assert order in range
	assert(order >= 0 && order <= LastOrder);

	// assert block_start aligned to order
	assert(block_aligned(order, block_start.pfn()));

	page *target = &block_start;
	page **candidate_slot = &free_list_[order];
	while (*candidate_slot && *candidate_slot != target) {
		candidate_slot = &((*candidate_slot)->next_free_);
	}

	// assert candidate block exists
	assert(*candidate_slot == target);

	*candidate_slot = target->next_free_;
	target->next_free_ = nullptr;
}

void page_allocator_buddy::split_block(int order, page &block_start) { panic("TODO"); }

void page_allocator_buddy::merge_buddies(int order, page &buddy) { panic("TODO"); }

page *page_allocator_buddy::allocate_pages(int order, page_allocation_flags flags) { panic("TODO"); }

void page_allocator_buddy::free_pages(page &block_start, int order) { panic("TODO"); }
