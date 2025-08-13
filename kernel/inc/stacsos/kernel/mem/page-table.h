/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/x86-page-table.h>

namespace stacsos::kernel::mem {
using page_table = arch::x86::x86_page_table;
using mapping_flags = arch::x86::mapping_flags;
using mapping_size = arch::x86::mapping_size;
using mapping_result = arch::x86::mapping_result;
using mapping = arch::x86::mapping;
} // namespace stacsos::kernel::mem
