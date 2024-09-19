/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/address-space.h>
#include <stacsos/kernel/mem/object-allocator.h>
#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page-table-allocator.h>

namespace stacsos::kernel::mem {
class memory_manager {
	DEFINE_SINGLETON(memory_manager)

private:
	memory_manager()
		: pgalloc_(nullptr)
		, root_address_space_(nullptr)
	{
	}

public:
	static void add_memory_block(u64 start, u64 length, bool avail);

	void init();

	page_allocator &pgalloc() const { return *pgalloc_; }

	page_table_allocator &ptalloc() { return ptalloc_; }
	const page_table_allocator &ptalloc() const { return ptalloc_; }

	object_allocator &objalloc() { return objalloc_; }
	const object_allocator &objalloc() const { return objalloc_; }

	address_space &root_address_space() const { return *root_address_space_; }

	bool try_handle_page_fault(u64 faulting_address);

private:
	void initialise_page_descriptors(u64 nr_page_descriptors);
	void initialise_page_allocator(u64 nr_page_descriptors);
	void initialise_object_allocator();
	void activate_primary_mapping();

	page_allocator *pgalloc_;
	page_table_allocator ptalloc_;
	object_allocator objalloc_;

	address_space *root_address_space_;
};
} // namespace stacsos::kernel::mem
