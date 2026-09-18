/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/page-alloc-ref.h>

namespace stacsos::kernel::mem {
using order_t = u32;

class page;
class memory_manager;

enum class page_allocation_flags { none = 0, zero = 1 };
DEFINE_ENUM_FLAG_OPERATIONS(page_allocation_flags)

enum class page_allocator_error { none, out_of_memory, order_out_of_range, page_not_aligned_with_order, not_implemented };

struct page_allocation_result {
public:
	static page_allocation_result ok(pfn_t range_start) { return page_allocation_result(range_start); }
	static page_allocation_result error(page_allocator_error e) { return page_allocation_result(e); }

	bool is_ok() const { return success; }
	bool is_error() const { return !success; }

	pfn_t get_range_start() const
	{
		if (is_error()) {
			panic("attempted to read range start from allocation failure");
		}

		return v.range_start;
	}

	page_allocator_error get_error() const
	{
		if (is_ok()) {
			panic("attempted to read error from allocation success");
		}

		return v.error;
	}

	page &to_page()
	{
		if (is_ok()) {
			return page::get_from_pfn(get_range_start());
		} else {
			switch (get_error()) {
			case page_allocator_error::out_of_memory:
				panic("out of memory");
				break;

			default:
				panic("allocation failed");
				break;
			}
		}
	}

private:
	page_allocation_result(pfn_t range_start)
		: success(true)
	{
		v.range_start = range_start;
	}

	page_allocation_result(page_allocator_error error)
		: success(false)
	{
		v.error = error;
	}

	bool success;
	union {
		pfn_t range_start;
		page_allocator_error error;
	} v;
};

struct page_allocator_stats {
	u64 used_pages, free_pages;
};

class page_allocator {
public:
	page_allocator(memory_manager &mm)
		: mm_(mm)
	{
	}

	virtual void insert_free_pages(pfn_t range_start, u64 page_count) = 0;

	virtual page_allocation_result allocate_pages(order_t order, page_allocation_flags flags = page_allocation_flags::none) = 0;
	virtual page_allocator_error free_pages(pfn_t range_start, order_t order) = 0;

	virtual page_allocator_stats get_stats() const = 0;

	virtual void dump() const = 0;

	void perform_selftest();

private:
	memory_manager &mm_;
};
} // namespace stacsos::kernel::mem
