/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/arch/x86/x2apic-timer.h>
#include <stacsos/kernel/arch/x86/x2apic.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/sleeper.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::sched;

void x2apic_timer::timer_irq_handler(u8 irq, void *context, void *arg)
{
	x2apic_timer *timer = (x2apic_timer *)arg;
	timer->lapic_.owner().update_accounting();

	sleeper::get().check_wakeup();

	timer->lapic_.owner().schedule();

	timer->lapic_.eoi();
}

void x2apic_timer::init() { lapic_.set_timer_irq(lapic_.owner().irqmgr().allocate_irq(timer_irq_handler, this)); }
