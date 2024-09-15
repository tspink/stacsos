/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/pio.h>

namespace stacsos::kernel::arch::x86 {
class pit {
public:
	void init_oneshot(u64 period)
	{
		u8 data = pio::inb(0x61);
		data &= 0xfe;
		data |= 1;

		// Set Gate Enable
		pio::outb(0x61, data);

		// Channel 2: lobyte/hibyte, hardware one-shot, binary mode
		pio::outb(0x43, 0xb2);

		// Set reload register
		pio::outb(0x42, period);
		pio::outb(0x42, period >> 8);
	}

	void go()
	{
		u8 data = pio::inb(0x61);

		// Clear Gate Enable
		data &= 0xfe;
		pio::outb(0x61, data);

		// Set Gate Enable
		data |= 1;
		pio::outb(0x61, data);
	}

	bool expired() const
	{
		u8 expired = pio::inb(0x61);
		return (expired & 0x20);
	}

	u64 frequency() const { return 1193180; }

	void spin(u64 milliseconds)
	{
		u64 millisecond_period = frequency() / 1000ull;
		init_oneshot(millisecond_period);

		for (u64 i = 0; i < milliseconds; i++) {
			go();

			while (!expired()) {
				__relax();
			}
		}
	}
};
} // namespace stacsos::kernel::arch::x86
