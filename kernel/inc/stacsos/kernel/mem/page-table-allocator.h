/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page.h>

namespace stacsos::kernel::mem {
class page_table_allocator {
public:
	page_table_allocator(page_allocator &pgalloc)
		: pgalloc_(pgalloc)
	{
	}

	page *allocate()
	{
		page *p = pgalloc_.allocate_pages(0, page_allocation_flags::zero);
		if (p == nullptr) {
			panic("unable to allocate page table");
		}

		return p;
	}

	void free(page *pg) { pgalloc_.free_pages(*pg, 0); }

private:
	page_allocator &pgalloc_;
};
} // namespace stacsos::kernel::mem
