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

class large_object_allocator {
public:
	large_object_allocator(void *region_base, size_t region_size)
		: region_base_(region_base)
		, base_(region_base)
		, size_(region_size)
	{
	}

	void *allocate(size_t size);
	bool free(void *ptr);

	bool ptr_in_region(void *ptr) const { return ((uintptr_t)ptr >= (uintptr_t)region_base_) && ((uintptr_t)ptr < ((uintptr_t)region_base_ + size_)); }

private:
	void *region_base_;
	void *base_;
	size_t size_;
};
} // namespace stacsos::kernel::mem
