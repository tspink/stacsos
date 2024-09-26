/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/mem/address-space-region.h>
#include <stacsos/kernel/mem/address-space.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-table-allocator.h>
#include <stacsos/kernel/mem/page-table.h>

using namespace stacsos::kernel::mem;

address_space *address_space::create_linked(u64 alloc_rgn_start)
{
	auto linked_pt = pt_->create_linked_copy(pta_);
	return new address_space(pta_, linked_pt, alloc_rgn_start);
}

address_space_region *address_space::alloc_region(u64 size, region_flags flags, bool allocate)
{
	u64 aligned_size = PAGE_ALIGN_UP(size);
	u64 base = next_alloc_rgn_;
	next_alloc_rgn_ += aligned_size;

	return add_region(base, size, flags, allocate);
}

address_space_region *address_space::add_region(u64 base, u64 size, region_flags flags, bool allocate)
{
	auto rgn = new address_space_region();
	rgn->base = base;
	rgn->size = size;
	rgn->flags = flags;

	//dprintf("as: add-region base=%lx size=%lx flags=%d alloc=%d\n", base, size, flags, allocate);

	if (allocate) {
		u64 pages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;
		rgn->storage = memory_manager::get().pgalloc().allocate_pages(log2_ceil(pages), page_allocation_flags::zero);

		u64 cur_virt = base;
		u64 cur_phys = rgn->storage->base_address();

		for (u64 i = 0; i < pages; i++) {
			//dprintf("map virt=%p phys=%p\n", cur_virt, cur_phys);
			pt_->map(pta_, cur_virt, cur_phys, mapping_flags::present | mapping_flags::writable | mapping_flags::user_accessable, mapping_size::m4k);
			cur_virt += PAGE_SIZE;
			cur_phys += PAGE_SIZE;
		}
	} else {
		rgn->storage = nullptr;
	}

	regions_.append(rgn);

	return rgn;
}

void address_space::remove_region(u64 base, u64 size, region_flags flags)
{
	//
}
