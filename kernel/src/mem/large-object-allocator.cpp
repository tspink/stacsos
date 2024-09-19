/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/mem/large-object-allocator.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/object-allocator.h>
#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page-table-allocator.h>
#include <stacsos/kernel/mem/page-table.h>
#include <stacsos/kernel/mem/page.h>

using namespace stacsos::kernel::mem;

/**
 * @brief Allocates a block of memory of the given size.
 *
 * @param size The size of memory to allocate
 * @return void* A pointer to the newly allocated memory, or zero if allocation failed.
 */
void *large_object_allocator::allocate(size_t size)
{
	auto &pga = memory_manager::get().pgalloc();
	auto &pta = memory_manager::get().ptalloc();

	// This is technically locked by the "object allocator" spin lock.

	u64 nr_pages = (size + PAGE_SIZE - 1) >> PAGE_BITS;
	u64 target = (u64)base_;

	page_table &v = memory_manager::get().root_address_space().pgtable();

	// We've computed the maximum number of pages needed to hold the allocation,
	// so for each bit in the number of pages required, allocate that order.
	// This works because, e.g. 3 pages = 0011 = order 1 (2) + order 0 (1)
	// And, e.g. 13 pages = 1101 = order 3 (8) + order 2 (4) + order 0 (1)

	// What we're doing is allocating physical pages for each order, then
	// "glueing" them together in the large object address space by inserting
	// appropriate mappings into the page table.

	// TODO: not sure how "free" is going to work... maybe some kind of
	// balanced tree holding the allocations?  Also, what if a page
	// allocation fails halfway through!?

	int pgi = 0; // The current monotonic page counter
	for (int i = 0; i < 32; i++) {
		// Only allocate when the bit is set
		if (nr_pages & (1 << i)) {
			page *pg = pga.allocate_pages(i); // Allocate a block of pages

			if (!pg) {
				panic("unable to allocate pages for object");
			}

			// For each page in the block...
			for (int j = 0; j < (1 << i); j++) {
				// Map the pages in this block into the virtual address space.
				v.map(pta, target + (PAGE_SIZE * pgi), pg->base_address() + (PAGE_SIZE * j), mapping_flags::writable);

				// Increase the current page counter.
				pgi++;
			}
		}
	}

	// HMM: vma_.activate();

	// Advance the base pointer by the number of pages we've just allocated (and mapped)
	base_ = (void *)((uintptr_t)base_ + (nr_pages * PAGE_SIZE));
	return (void *)target;
}

/**
 * @brief Frees a block of memory allocated with the corresponding allocate function.
 *
 * @param p A pointer to the block of memory (allocated by allocated), which is to be freed.
 */
bool large_object_allocator::free(void *p)
{
	if (!ptr_in_region(p)) {
		return false;
	}

	// TODO

	return true;
}
