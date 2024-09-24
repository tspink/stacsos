/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/sched/event.h>
#include <stacsos/kernel/sched/schedulable-entity.h>

namespace stacsos::kernel::mem {
class page;
}

namespace stacsos::kernel::arch {
class core;
}

namespace stacsos::kernel::sched {
using namespace stacsos::kernel::arch;

enum class thread_states { created, runnable, running, suspended, terminated };

class process;

class thread : public schedulable_entity {
public:
	static const int stack_size_order = 4;
	static const size_t stack_size = (1 << stack_size_order) * PAGE_SIZE;

	thread(process &owner, u64 ep = 0, void *ep_arg = nullptr, u64 user_stack = 0);

	thread_states state() const { return state_; }
	event &state_changed_event() { return state_changed_event_; }

	void start();
	void stop();
	void suspend();
	void resume();

	process &owner() const { return owner_; }

	static thread &current();

private:
	static __noreturn void task_entry_trampoline(thread *task);
	void init_tcb();
	bool is_self() const;
	void change_state(thread_states new_state);

	process &owner_;
	u64 ep_;
	void *arg_;
	thread_states state_;
	mem::page *kernel_stack_;
	u64 user_stack_;
	event state_changed_event_;
};
} // namespace stacsos::kernel::sched
