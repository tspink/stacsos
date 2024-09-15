/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/alg/sfs.h>
#include <stacsos/kernel/sched/schedulable-entity.h>

using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::sched::alg;

tcb *simple_fair_scheduler::select_next_task(tcb *current)
{
	if (runqueue_.empty()) {
		return nullptr;
	}

	if (runqueue_.count() == 1) {
		return runqueue_.first();
	}

	u64 min_runtime = 0;
	tcb *candidate = nullptr;

	for (auto *thread : runqueue_) {
		if (candidate == nullptr || (thread->run_time < min_runtime)) {
			min_runtime = thread->run_time;
			candidate = thread;
		}
	}

	return candidate;
}
