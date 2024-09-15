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

void event::wait()
{
	// dprintf("event %p wait\n", this);

	thread *ct = &thread::current();

	wait_list_.append(ct);
	ct->suspend();

	asm volatile("int $0xff");
}

void event::trigger()
{
	// dprintf("event %p trigger\n", this);

	for (auto thread : wait_list_) {
		thread->resume();
	}

	wait_list_.clear();
}
