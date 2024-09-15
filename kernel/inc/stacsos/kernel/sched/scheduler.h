/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::sched {
class schedulable_entity;

class scheduler {
	DEFINE_SINGLETON(scheduler);

private:
	scheduler() { }

public:
	void add_to_schedule(schedulable_entity &e);
	void remove_from_schedule(schedulable_entity &e);
};
} // namespace stacsos::kernel::sched
