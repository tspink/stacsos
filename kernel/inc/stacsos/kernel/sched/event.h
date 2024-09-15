/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/list.h>

namespace stacsos::kernel::sched {
class thread;

class event {
public:
	void trigger();
	void wait();

private:
	list<thread *> wait_list_;
};
} // namespace stacsos::kernel::sched
