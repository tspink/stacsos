/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/object-allocator.h>
#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page.h>

using namespace stacsos::kernel::mem;

#define VMALLOC_AREA 0xfffff00000000000

object_allocator::object_allocator()
	: loa_((void *)VMALLOC_AREA, GB(1))
{
}

void *object_allocator::alloc(size_t size)
{
	unique_irq_lock l(object_allocator_lock_);

	if (size <= 16) {
		return cache16_.allocate();
	} else if (size <= 32) {
		return cache32_.allocate();
	} else if (size <= 64) {
		return cache64_.allocate();
	} else if (size <= 128) {
		return cache128_.allocate();
	} else if (size <= 256) {
		return cache256_.allocate();
	} else if (size <= 512) {
		return cache512_.allocate();
	} else if (size <= 1024) {
		return cache1024_.allocate();
	}

	return loa_.allocate(size);
}

void object_allocator::free(void *ptr)
{
	unique_irq_lock l(object_allocator_lock_);

	if (loa_.ptr_in_region(ptr)) {
		if (!loa_.free(ptr)) {
			panic("unable to free large object");
		}
	} else {
		if (cache16_.try_free(ptr))
			return;
		if (cache32_.try_free(ptr))
			return;
		if (cache64_.try_free(ptr))
			return;
		if (cache128_.try_free(ptr))
			return;
		if (cache256_.try_free(ptr))
			return;
		if (cache512_.try_free(ptr))
			return;
		if (cache1024_.try_free(ptr))
			return;

		panic("unable to free object");
	}
}
