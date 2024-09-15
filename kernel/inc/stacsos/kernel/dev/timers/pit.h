/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/dev/timers/timer.h>

namespace stacsos::kernel::dev::timers {
using namespace stacsos::kernel::arch::x86;

class pit : public timer {
public:
	static device_class pit_device_class;

	pit(bus &parent)
		: timer(pit_device_class, parent)
	{
	}

	virtual void configure() override;

	void init_oneshot(u64 period);

	void go();

	bool expired() const
	{
		u8 expired = ioport<0x61>::read8();
		return !(expired & 0x20);
	}

	u64 frequency() const { return 1193180; }
};
} // namespace stacsos::kernel::dev::timers
