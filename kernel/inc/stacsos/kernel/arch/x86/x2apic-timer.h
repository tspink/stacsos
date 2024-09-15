/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/timer.h>
#include <stacsos/kernel/arch/x86/x2apic.h>

namespace stacsos::kernel::arch::x86 {

class x2apic_timer : public timer {
public:
	x2apic_timer(x2apic &lapic)
		: lapic_(lapic)
	{
	}

	virtual ~x2apic_timer() { }

	virtual void init() override;

	virtual void start(u64 frequency)
	{
		lapic_.set_timer_periodic();
		lapic_.set_timer_divide(3);
		lapic_.set_timer_initial_count((lapic_.get_timer_frequency() >> 4) / frequency);

		lapic_.unmask_interrupts(x2apic_lvts::timer);
	}

	virtual void stop() { lapic_.mask_interrupts(x2apic_lvts::timer); }

private:
	static void timer_irq_handler(u8 irq, void *context, void *arg);
	x2apic &lapic_;
};
} // namespace stacsos::kernel::arch::x86
