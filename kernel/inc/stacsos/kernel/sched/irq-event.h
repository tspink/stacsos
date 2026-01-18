/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/lock.h>
#include <stacsos/list.h>

namespace stacsos::kernel::sched {
class thread;

class irq_event {
public:
	irq_event()
		: triggered_(false)
	{
	}

	void trigger() { triggered_ = true; }
	void reset() { triggered_ = false; }

	void wait()
	{
		// Spin
		while (!triggered_) {
			asm volatile("pause");
		}
	}

private:
	volatile bool triggered_;
	//	list<thread *> wait_list_;
};

} // namespace stacsos::kernel::sched
