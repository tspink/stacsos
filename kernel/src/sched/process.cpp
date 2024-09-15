/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/mem/address-space.h>
#include <stacsos/kernel/sched/process.h>
#include <stacsos/kernel/sched/thread.h>

using namespace stacsos;
using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::mem;

shared_ptr<thread> process::create_thread(u64 entry_point, void *entry_arg)
{
	u64 user_stack = 0;
	if (priv_ == exec_privilege::user) {
		u64 stack_base = next_user_stack_;
		u64 stack_size = 0x4000;

		next_user_stack_ += stack_size + 0x1000; // Allocate the stack size, but plus a "guard page".

		user_stack = stack_base + stack_size;
		addrspace().add_region(stack_base, stack_size, region_flags::readwrite, true);
	}

	shared_ptr<thread> t = shared_ptr(new thread(*this, entry_point, entry_arg, user_stack));
	threads_.append(t);

	return t;
}

void process::start()
{
	for (auto &t : threads_) {
		t->start();
	}

	state_ = process_state::started;
	state_changed_event_.trigger();
}

void process::stop()
{
	for (auto &t : threads_) {
		t->stop();
	}

	state_ = process_state::terminated;
	state_changed_event_.trigger();
}

void process::on_thread_stopped(thread &thread)
{
	dprintf("thread stopped\n");

	for (auto &t : threads_) {
		if (t->state() != thread_states::terminated) {
			return;
		}
	}

	dprintf("process terminated\n");
	state_ = process_state::terminated;
	state_changed_event_.trigger();
}
