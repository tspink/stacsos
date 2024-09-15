/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch {
class irq {
public:
	using irq_handler_fn = void (*)(void *);
	void attach(irq_handler_fn handler, void *arg);
};
} // namespace stacsos::kernel::arch
