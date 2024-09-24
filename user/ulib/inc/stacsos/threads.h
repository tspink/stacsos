/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
typedef void *(*thread_entry_fn)(void *);

struct thread_context {
	thread_entry_fn ep_;
	void *arg_;
	void *result_;
};

class thread {
public:
	~thread() { delete tc_; }

	static thread *start(thread_entry_fn ep, void *arg = nullptr);

	void *join();

private:
	thread(u64 handle, thread_context *tc)
		: handle_(handle)
		, tc_(tc)
	{
	}

	u64 handle_;
	thread_context *tc_;
};
} // namespace stacsos
