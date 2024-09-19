/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/lock.h>
#include <stacsos/kernel/mem/large-object-allocator.h>
#include <stacsos/kernel/mem/slab-cache.h>

namespace stacsos::kernel::mem {
class memory_manager;

class object_allocator {
public:
	object_allocator();

	void *alloc(size_t size);
	void *realloc(void *obj, size_t size);
	void free(void *obj);

private:
	spinlock_irq object_allocator_lock_;

	slab_cache<16, 0> cache16_;
	slab_cache<32, 0> cache32_;
	slab_cache<64, 0> cache64_;
	slab_cache<128, 0> cache128_;
	slab_cache<256, 0> cache256_;
	slab_cache<512, 0> cache512_;
	slab_cache<1024, 0> cache1024_;
	large_object_allocator loa_;
};
} // namespace stacsos::kernel::mem
