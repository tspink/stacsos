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

struct sleeping_thread {
	thread *thr;
	u64 wakeup_deadline;
};

class sleeper {
	DEFINE_SINGLETON(sleeper)

public:
	void sleep_ms(u64 duration_ms);
	void check_wakeup();

private:
	sleeper() { }

	list<sleeping_thread *> sleeping_;

	void do_sleep(u64 wakeup_deadline);
};
} // namespace stacsos::kernel::sched
