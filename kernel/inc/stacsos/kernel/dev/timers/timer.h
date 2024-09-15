/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::timers {
class timer : public device {
public:
	static device_class timer_device_class;

	timer(device_class &devclass, bus &parent)
		: device(devclass, parent)
	{
	}
};
} // namespace stacsos::kernel::dev::timers
