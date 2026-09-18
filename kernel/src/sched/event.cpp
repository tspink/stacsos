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
	u64 flags;

	lock_.lock(&flags);
	if (!AUTO_RESET && triggered_) {
		lock_.unlock(flags);
		return;
	}

	thread *ct = &thread::current();

	wait_list_.append(ct);
	ct->suspend();

	lock_.unlock(flags);

	asm volatile("int $0xff");
}

template <bool AUTO_RESET> void event<AUTO_RESET>::trigger()
{
	u64 flags;
	lock_.lock(&flags);

	if (!AUTO_RESET) {
		triggered_ = true;
	}

	// TODO: This should only release ONE thread if it's an auto reset event.
	// Probably need some kind of mutex too.
	for (auto thread : wait_list_) {
		thread->resume();
	}

	wait_list_.clear();

	lock_.unlock(flags);
}

template class event<true>;
template class event<false>;
