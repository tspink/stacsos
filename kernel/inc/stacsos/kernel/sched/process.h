/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/mem/address-space.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/sched/event.h>
#include <stacsos/kernel/sched/thread.h>
#include <stacsos/list.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::sched {
class thread;

enum exec_privilege { kernel, user };

enum class process_state { created, started, terminated };

class process {
	friend class thread;

public:
	process(exec_privilege priv)
		: priv_(priv)
		, state_(process_state::created)
		, vma_(mem::memory_manager::get().root_address_space().create_linked(0x7fff'2000'0000))
		, next_user_stack_(0x7fff'1000'0000)
	{
	}

	exec_privilege privilege() const { return priv_; }

	shared_ptr<thread> create_thread(u64 entry_point, void *entry_arg = nullptr);

	process_state state() const { return state_; }

	void start();
	void stop();

	mem::address_space &addrspace() const { return *vma_; }

	event &state_changed_event() { return state_changed_event_; }

private:
	exec_privilege priv_;
	process_state state_;
	event state_changed_event_;

	mem::address_space *vma_;
	list<shared_ptr<thread>> threads_;
	u64 next_user_stack_;

	void on_thread_stopped(thread &thread);
};
} // namespace stacsos::kernel::sched
