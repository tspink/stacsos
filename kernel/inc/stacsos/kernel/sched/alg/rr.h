/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/sched/alg/scheduling-algorithm.h>

namespace stacsos::kernel::sched::alg {

// *** COURSEWORK NOTE *** //
// Please edit this file in any way you see fit.

class round_robin : public scheduling_algorithm {
public:
	virtual void add_to_runqueue(tcb &tcb) override;
	virtual void remove_from_runqueue(tcb &tcb) override;
	virtual tcb *select_next_task(tcb *current) override;
	virtual const char *name() const { return "round robin"; }
};
} // namespace stacsos::kernel::sched::alg
