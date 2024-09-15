/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/irq.h>

namespace stacsos::kernel::arch {
class core;

class irq_manager {
public:
	irq_manager(core &core)
		: core_(core)
	{
	}

	irq *request_irq();

private:
	core &core_;
};
} // namespace stacsos::kernel::arch
