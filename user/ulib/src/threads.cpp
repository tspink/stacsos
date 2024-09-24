/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/threads.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

static void thread_entry_proc(thread_context *tc)
{
	tc->result_ = tc->ep_(tc->arg_);
	syscalls::stop_current_thread();
}

thread *thread::start(thread_entry_fn ep, void *arg)
{
	auto tc = new thread_context { ep, arg, nullptr };

	auto r = syscalls::start_thread((void *)thread_entry_proc, tc);
	if (r.code != syscall_result_code::ok) {
		return nullptr;
	}

	return new thread(r.data, tc);
}

void *thread::join()
{
	auto r = syscalls::join_thread(handle_);
	return tc_->result_;
}
