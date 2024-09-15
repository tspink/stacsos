/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch {
typedef void (*timer_callback)(void *);
class timer {
public:
	timer()
		: cb_(nullptr)
		, cb_arg_(nullptr)
	{
	}

	virtual ~timer() { }

	virtual void init() = 0;

	void set_callback(timer_callback cb, void *arg = nullptr)
	{
		cb_ = cb;
		cb_arg_ = arg;
	}

	virtual void start(u64 period) = 0;
	virtual void stop() = 0;

private:
	timer_callback cb_;
	void *cb_arg_;
};
} // namespace stacsos::kernel::arch
