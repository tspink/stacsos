/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/sched/process.h>
#include <stacsos/list.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::sched {
typedef void (*continuation_fn)(void);

class process_manager {
	friend class process;

public:
	DEFINE_SINGLETON(process_manager)

	process_manager() { }

	void init();

	shared_ptr<process> create_kernel_process(continuation_fn ep);
	shared_ptr<process> create_process(const char *path, const char *args);

private:
	list<shared_ptr<process>> active_processes_;
};
} // namespace stacsos::kernel::sched
