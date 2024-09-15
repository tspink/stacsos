/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/tsc.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/sleeper.h>
#include <stacsos/kernel/sched/thread.h>

using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::arch::x86;

void sleeper::sleep_ms(u64 duration_ms)
{
	auto &tsc = x86_core::this_core().local_tsc();
	u64 ref_time = tsc.read();

	do_sleep(ref_time + ((duration_ms * tsc.frequency()) / 1000));
}

void sleeper::do_sleep(u64 wakeup_deadline)
{
	thread *ct = &thread::current();
	ct->suspend();

	sleeping_thread *st = new sleeping_thread { ct, wakeup_deadline };
	sleeping_.append(st);

	// dprintf("sleeper: sleeping %p deadline=%lu\n", ct, wakeup_deadline);

	asm volatile("int $0xff");
}

void sleeper::check_wakeup()
{
	u64 ref_time = x86_core::this_core().local_tsc().read();

	// TODO: some kind of priority queue
	list<sleeping_thread *> resumed;
	for (auto sleeping : sleeping_) {
		if (ref_time > sleeping->wakeup_deadline) {
			// dprintf("sleeper: waking %p\n", sleeping->thr);
			sleeping->thr->resume();
			resumed.append(sleeping);
		}
	}

	for (auto resume : resumed) {
		sleeping_.remove(resume);
	}
}
