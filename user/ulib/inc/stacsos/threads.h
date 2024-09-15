/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
typedef void *(*thread_entry_fn)(void *);

class thread {
public:
	static thread *start(thread_entry_fn ep, void *arg = nullptr);

	void *join();

private:
	thread(u64 handle)
		: handle_(handle)
	{
	}

	u64 handle_;
};
} // namespace stacsos
