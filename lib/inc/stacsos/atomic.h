/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
template <class T> class atomic {
public:
	using self = atomic<T>;

	atomic(T v)
		: v_(v)
	{
	}

	T fetch_and_add(T value)
	{
		asm volatile("lock; xadd %0, %1"
					 : "+r"(value), "+m"(v_) // input + output
					 : // No input-only
					 : "memory");

		return value;
	}

	T operator++(int) { return fetch_and_add(1); }

	self &operator=(T value)
	{
		v_ = value;
		return *this;
	}

private:
	T v_;
};

using atomic_u32 = atomic<u32>;
using atomic_u64 = atomic<u64>;
} // namespace stacsos
