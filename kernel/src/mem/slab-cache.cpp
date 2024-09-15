/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/mem/page.h>
#include <stacsos/kernel/mem/slab-cache.h>

using namespace stacsos::kernel::mem;

template <size_t object_size, int slab_page_order> void *slab_cache<object_size, slab_page_order>::allocate_slab()
{
	return (pgalloc_.allocate_pages(slab_page_order))->base_address_ptr();
}

template class slab_cache<16, 0>;
template class slab_cache<32, 0>;
template class slab_cache<64, 0>;
template class slab_cache<128, 0>;
template class slab_cache<256, 0>;
template class slab_cache<512, 0>;
template class slab_cache<1024, 0>;
