/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/event.h>
#include <stacsos/kernel/sched/thread.h>

using namespace stacsos::kernel::sched;

template <bool AUTO_RESET> void event<AUTO_RESET>::wait()
{
	if (!AUTO_RESET && triggered_) {
		return;
	}

	// TODO: Race Condition!

	thread *ct = &thread::current();

	wait_list_.append(ct);
	ct->suspend();

	asm volatile("int $0xff");
}

template <bool AUTO_RESET> void event<AUTO_RESET>::trigger()
{
	if (!AUTO_RESET) {
		triggered_ = true;
	}

	for (auto thread : wait_list_) {
		thread->resume();
	}

	wait_list_.clear();
}

template class event<true>;
template class event<false>;
