/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/address-space-region.h>
#include <stacsos/kernel/mem/page-table.h>
#include <stacsos/list.h>

namespace stacsos::kernel::mem {
class page_table_allocator;
class memory_manager;

class address_space {
	friend class memory_manager;

public:
	address_space(page_table_allocator &pta, u64 alloc_rgn_start)
		: pta_(pta)
		, pt_(page_table::create_empty(pta))
		, next_alloc_rgn_(alloc_rgn_start)
	{
	}

	~address_space() { panic("free addrspace"); }

	page_table &pgtable() const { return *pt_; }

	address_space_region *alloc_region(u64 size, region_flags flags, bool allocate);
	address_space_region *add_region(u64 base, u64 size, region_flags flags, bool allocate);
	void remove_region(u64 base, u64 size, region_flags flags);

	address_space_region *get_region_from_address(u64 address)
	{
		for (address_space_region *rgn : regions_) {
			if (address >= rgn->base && address < (rgn->base + rgn->size)) {
				return rgn;
			}
		}

		return nullptr;
	}

	address_space *create_linked(u64 alloc_rgn_start);

private:
	address_space(page_table_allocator &pta, page_table *pt, u64 alloc_rgn_start)
		: pta_(pta)
		, pt_(pt)
		, next_alloc_rgn_(alloc_rgn_start)
	{
	}

	page_table_allocator &pta_;
	page_table *pt_;

	list<address_space_region *> regions_;
	u64 next_alloc_rgn_;
};
} // namespace stacsos::kernel::mem
