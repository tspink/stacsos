/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024, 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/page-allocator-buddy.h>
#include <stacsos/kernel/mem/page.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::mem;

// Represents the contents of a free page, that can hold useful metadata.
struct page_metadata {
	page *next_free;
};

/**
 * @brief Dumps out (via the debugging routines) the current state of the buddy page allocator's free lists
 */
void page_allocator_buddy::dump() const
{
	// Print out a header, so we can quickly identify this output in the debug stream.
	dprintf("*** buddy page allocator - free list ***\n");

	// Loop over each order that our allocator is responsible for, from zero up to *and
	// including* LastOrder.
	for (int order = 0; order <= LastOrder; order++) {
		// Print out the order number (with a leading zero, so that it's nicely aligned)
		dprintf("[%02u] ", order);

		// Get the pointer to the first free page in the free list.
		page *current_free_page = free_list_[order];

		// While there /is/ currently a free page in the list...
		while (current_free_page) {
			// Print out the extents of this page, i.e. its base address (at byte granularity), up to and including the last
			// valid address.  Remember: these are PHYSICAL addresses.
			dprintf("%lx--%lx ", current_free_page->base_address(), (current_free_page->base_address() + ((1 << order) << PAGE_BITS)) - 1);

			// Advance to the next page, by interpreting the free page as holding metadata, and reading
			// the appropriate field.
			current_free_page = ((page_metadata *)current_free_page->base_address_ptr())->next_free;
		}

		// New line for the next order.
		dprintf("\n");
	}
}

/**
 * @brief Inserts pages that are known to be free into the buddy allocator.
 *
 * ** You are required to implement this function **
 *
 * @param range_start The first page in the range.
 * @param page_count The number of pages in the range.
 */
void page_allocator_buddy::insert_free_pages(page &range_start, u64 page_count) { panic("TODO"); }

/**
 * @brief
 *
 * @param order
 * @param block_start
 */
void page_allocator_buddy::insert_free_block(int order, page &block_start)
{
	// assert order in range
	assert(order >= 0 && order <= LastOrder);

	// assert block_start aligned to order
	assert(block_aligned(order, block_start.pfn()));

	page *target = &block_start;
	page **slot = &free_list_[order];
	while (*slot && *slot < target) {
		// slot = &((*slot)->next_free_);
		slot = &((page_metadata *)((*slot)->base_address_ptr()))->next_free;
	}

	assert(*slot != target);

	((page_metadata *)target->base_address_ptr())->next_free = *slot;
	*slot = target;
}

/**
 * @brief
 *
 * @param order
 * @param block_start
 */
void page_allocator_buddy::remove_free_block(int order, page &block_start)
{
	// assert order in range
	assert(order >= 0 && order <= LastOrder);

	// assert block_start aligned to order
	assert(block_aligned(order, block_start.pfn()));

	page *target = &block_start;
	page **candidate_slot = &free_list_[order];
	while (*candidate_slot && *candidate_slot != target) {
		candidate_slot = &((page_metadata *)(*candidate_slot)->base_address_ptr())->next_free; // &((*candidate_slot)->next_free_);
	}

	// assert candidate block exists
	assert(*candidate_slot == target);

	*candidate_slot = ((page_metadata *)target->base_address_ptr())->next_free;
	((page_metadata *)target->base_address_ptr())->next_free = nullptr;

	// target->next_free_ = nullptr;
}

/**
 * @brief
 *
 * ** You are required to implement this function **
 * @param order
 * @param block_start
 */
void page_allocator_buddy::split_block(int order, page &block_start) { panic("TODO"); }

/**
 * @brief
 *
 * @param order
 * @param buddy
 */
void page_allocator_buddy::merge_buddies(int order, page &buddy) { panic("TODO"); }

/**
 * @brief
 *
 * ** You are required to implement this function **
 * @param order
 * @param flags
 * @return page*
 */
page *page_allocator_buddy::allocate_pages(int order, page_allocation_flags flags) { panic("TODO"); }

/**
 * @brief
 *
 * ** You are required to implement this function **
 * @param block_start
 * @param order
 */
void page_allocator_buddy::free_pages(page &block_start, int order) { panic("TODO"); }
