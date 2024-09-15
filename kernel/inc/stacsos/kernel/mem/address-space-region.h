/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::mem {
class page;

enum class region_flags { inaccessible = 0, readable = 1, writable = 2, executable = 4, readwrite = 3, all = 7 };

DEFINE_ENUM_FLAG_OPERATIONS(region_flags)

class address_space_region {
public:
	u64 base, size;
	region_flags flags;
	page *storage;
};
} // namespace stacsos::kernel::mem
