/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/threads.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

thread *thread::start(thread_entry_fn ep, void *arg)
{
	auto r = syscalls::start_thread((void *)ep, arg);

	if (r.code != syscall_result_code::ok) {
		return nullptr;
	}

	return new thread(r.data);
}

void *thread::join()
{
	auto r = syscalls::join_thread(handle_);
	return (void *)r.data;
}
