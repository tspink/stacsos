/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/bus.h>

namespace stacsos::kernel::arch {
class platform : public dev::bus {
public:
	platform()
		: dev::bus()
	{
	}
};
} // namespace stacsos::kernel::arch
