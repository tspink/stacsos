/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page-table-allocator.h>

namespace stacsos::kernel::mem {
struct object_header {
	u64 size;
	u8 data[];
};

class object_allocator;

class large_object_allocator {
public:
	large_object_allocator(object_allocator &objalloc, page_allocator &pgalloc, page_table_allocator &ptalloc, void *region_base, size_t region_size)
		: objalloc_(objalloc)
		, pgalloc_(pgalloc)
		, ptalloc_(ptalloc)
		, region_base_(region_base)
		, base_(region_base)
		, size_(region_size)
	{
	}

	void *allocate(size_t size);
	bool free(void *ptr);

	bool ptr_in_region(void *ptr) const { return ((uintptr_t)ptr >= (uintptr_t)region_base_) && ((uintptr_t)ptr < ((uintptr_t)region_base_ + size_)); }

private:
	object_allocator &objalloc_;
	page_allocator &pgalloc_;
	page_table_allocator &ptalloc_;

	void *region_base_;
	void *base_;
	size_t size_;
};
} // namespace stacsos::kernel::mem
