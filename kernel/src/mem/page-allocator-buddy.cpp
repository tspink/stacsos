/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024, 2025, 2026
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
	pfn_t next_free;

	/**
	 * @brief Checks to see if the next_free pointer is valid or not.  Note, this actually
	 * only checks to see if the next_free pointer is /invalid/ -- it doesn't necessarily
	 * guarantee that a pointer that looks valid *is* valid.
	 *
	 * @return false Returns false if the next_free point is invalid.
	 * @return true Returns true otherwise.
	 */
	bool has_next_free() const { return next_free != INVALID_PFN; }
};

/**
 * @brief Returns a pointer to the metadata structure that is held within a free page.  This CANNOT be used on
 * pages that have been allocated, as they are owned by the requesting code.  Once pages have been freed, or
 * are being returned to the allocator, this metadata can be used.
 *
 * @param pfn The PFN of the page on which to retrieve the metadata struct.
 * @return page_metadata* The metadata structure.
 */
static inline page_metadata *metadata(pfn_t pfn) { return (page_metadata *)page::get_from_pfn(pfn).base_address_ptr(); }

/**
 * @brief Dumps out (via the debugging routines) the current state of the buddy allocator's free lists.
 */
void page_allocator_buddy::dump() const
{
	// Print out a header, so we can quickly identify this output in the debug stream.
	dprintf("*** buddy page allocator - free list ***\n");

	// Loop over each order that our allocator is responsible for, from zero up to *and
	// including* LastOrder.
	for (order_t order = 0; order <= LAST_ORDER; order++) {
		// Print out the order number (with a leading zero, so that it's nicely aligned)
		dprintf("[%02u] ", order);

		// If the free list is empty -- print out a message and continue to the next order.
		if (free_list_[order] == INVALID_PFN) {
			dprintf("(empty)\n");
			continue;
		}

		// Get the pointer to the first free page in the free list.
		pfn_t current_free_page = free_list_[order];

		// While there /is/ currently a free page in the list...
		do {
			if (order == 0) {
				// If this is order-0, then there's only one page in the free block, so no need
				// to print out a range.
				dprintf("%lu ", current_free_page);
			} else {
				// Otherwise, print out the extents of this free block, using the PFN of the
				// starting page, up to and INCLUDING the PFN of the last page in the block.
				dprintf("%lu--%lu ", current_free_page, current_free_page + ((1 << order) - 1));
			}

			// Break out of the loop if there isn't a next page in the list.
			if (!metadata(current_free_page)->has_next_free()) {
				break;
			}

			// Advance to the next page, by interpreting the free page as holding metadata,
			// and reading the appropriate field.
			current_free_page = metadata(current_free_page)->next_free;
		} while (true);

		// New line for the next order.
		dprintf("\n");
	}

	dprintf("stats: free=%lu, used=%lu\n", get_stats().free_pages, get_stats().used_pages);
}

/**************************************************************************************/
/* The following functions are part of the private API, and have been implemented for */
/* you.  You are not required to use them, but you may find them very useful.         */
/**************************************************************************************/

/**
 * @brief Inserts a block of pages into the free list for the given order.
 *
 * @param order The order in which to insert the free block.
 * @param block_start The PFN of the starting page of the block to be inserted.
 */
void page_allocator_buddy::insert_free_block(order_t order, pfn_t block_start)
{
	// Assert that the given order is in the range of orders we support.
	assert(order >= 0 && order <= LAST_ORDER);

	// Assert that the starting page in the block is aligned to the requested order.
	assert(block_aligned(order, block_start));

	// Iterate through the free list, until we get to the position where the
	// block should be inserted, i.e. ordered by page frame number.
	pfn_t target = block_start;

	if (free_list_[order] == INVALID_PFN) {
		// If the free list is empty, update the list head.
		free_list_[order] = block_start;

		// And, make sure the page metadata holds an invalid PFN.
		metadata(block_start)->next_free = INVALID_PFN;
	} else {
		// Otherwise, iterate until we find the right position.
		pfn_t current = free_list_[order];
		pfn_t prev = INVALID_PFN;
		do {
			if (current != INVALID_PFN && block_start > current) {
				prev = current;
				current = metadata(current)->next_free;
			} else {
				// And, once we're in the right place, insert the block.
				if (prev == INVALID_PFN) {
					free_list_[order] = block_start;
				} else {
					metadata(prev)->next_free = block_start;
				}

				metadata(block_start)->next_free = current;
				break;
			}
		} while (1);
	}
}

/**
 * @brief Removes a block of pages from the free list of the specified order.
 *
 * @param order The order in which to remove a free block.
 * @param block_start The PFN of the starting page of the block to be removed.
 */
void page_allocator_buddy::remove_free_block(order_t order, pfn_t block_start)
{
	// Assert that the given order is in the range of orders we support.
	assert(order >= 0 && order <= LAST_ORDER);

	// Assert that the starting page in the block is aligned to the requested order.
	assert(block_aligned(order, block_start));

	pfn_t current = free_list_[order];
	pfn_t prev = INVALID_PFN;

	// Loop through blocks in the free list, until we find the one we want.
	while (current != block_start) {
		prev = current;
		current = metadata(current)->next_free;
	}

	// Check that we /actually/ found the one we want.
	if (current != block_start) {
		panic("block not in free list");
	}

	// Remove from the list (taking care to modify the correct slot).
	if (prev == INVALID_PFN) {
		free_list_[order] = metadata(current)->next_free;
	} else {
		metadata(prev)->next_free = metadata(current)->next_free;
	}

	metadata(current)->next_free = INVALID_PFN;
}

/*************************************************************************************/
/* The following functions are part of the private API, and so only need to be       */
/* implemented if you plan on using them.  If you do not use them, then remove them. */
/*************************************************************************************/

/**
 * @brief Splits a free block of pages from a given order, into two halves into a lower order.
 *
 * ** You are required to implement this function **
 * @param order The order in which the free block current exists.
 * @param block_start The starting page of the block to be split.
 */
void page_allocator_buddy::split_block(order_t order, pfn_t block_start) { panic("TODO"); }

/**
 * @brief Merges two buddy-adjacent free blocks in one order, into a block in the next higher order.
 *
 * ** You are required to implement this function **
 * @param order The order in which to merge buddies.
 * @param either_buddy Either buddy page in the free block.
 */
void page_allocator_buddy::merge_buddies(order_t order, pfn_t either_buddy) { panic("TODO"); }

/************************************************************************************/
/* The following functions are part of the public API, and so /must/ be implemented */
/************************************************************************************/

/**
 * @brief Inserts pages that are known to be free into the buddy allocator.
 *
 * ** You are required to implement this function **
 *
 * @param range_start The first page in the range.
 * @param page_count The number of pages in the range.
 */
void page_allocator_buddy::insert_free_pages(pfn_t range_start, u64 page_count) { panic("TODO"); }

/**
 * @brief Allocates pages, using the buddy algorithm.
 *
 * ** You are required to implement this function **
 * @param order The order of pages to allocate (i.e. 2^order number of pages)
 * @param flags Any allocation flags to take into account.
 * @return page* The starting page of the block that was allocated, or nullptr if the allocation cannot be satisfied.
 */
page_allocation_result page_allocator_buddy::allocate_pages(order_t order, page_allocation_flags flags) { panic("TODO"); }

/**
 * @brief Frees previously allocated pages, using the buddy algorithm.
 *
 * ** You are required to implement this function **
 * @param block_start The starting page of the block to be freed.
 * @param order The order of the block being freed.
 */
page_allocator_error page_allocator_buddy::free_pages(pfn_t range_start, order_t order) { panic("TODO"); }

/**
 * @brief Returns the current state of the page allocator.
 *
 * ** You are required to implement this function **
 * @return page_allocator_stats A structure containing the state of the allocator.
 */
page_allocator_stats page_allocator_buddy::get_stats() const { panic("TODO"); }
