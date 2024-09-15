/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::sched {
class tcb;
}

namespace stacsos::kernel::sched::alg {
class scheduling_algorithm {
public:
	virtual void add_to_runqueue(tcb &tcb) = 0;
	virtual void remove_from_runqueue(tcb &tcb) = 0;
	virtual tcb *select_next_task(tcb *current) = 0;
	virtual const char *name() const = 0;
};
} // namespace stacsos::kernel::sched::alg
