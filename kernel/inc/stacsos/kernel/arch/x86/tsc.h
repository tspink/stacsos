/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {
class tsc {
public:
	tsc()
		: timer_frequency_(0)
	{
	}

	void calibrate();

	u64 read()
	{
		u32 pid;
		return __builtin_ia32_rdtscp(&pid);
	}

	u64 frequency() const { return timer_frequency_; }

	void spin(u64 milliseconds)
	{
		u64 millisecond_period = (frequency() * milliseconds) / 1000ull;
		u64 target = read() + millisecond_period;

		while (read() < target) {
			__relax();
		}
	}

private:
	u64 timer_frequency_;
};
} // namespace stacsos::kernel::arch::x86
