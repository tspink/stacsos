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
	page *allocate();
	void free(page *pg);
};
} // namespace stacsos::kernel::mem
