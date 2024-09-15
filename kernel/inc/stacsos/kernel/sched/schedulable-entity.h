/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/memops.h>

namespace stacsos::kernel::arch {
class core;
}

namespace stacsos::kernel::sched {
class schedulable_entity;

struct tcb {
	schedulable_entity *entity; // 0
	stacsos::kernel::arch::x86::machine_context *mcontext; // 8
	u64 cr3; // 10
	u64 kernel_stack; // 18
	u64 user_stack_save; // 20
	u64 start_time;	// 28
	u64 stop_time;	// 30
	u64 run_time;	// 38
} __packed;

class schedulable_entity {
public:
	schedulable_entity()
		: owning_core_(nullptr)
	{
		memops::bzero(&tcb_, sizeof(tcb_));
	}

	const tcb *get_tcb() const { return &tcb_; }
	tcb *get_tcb() { return &tcb_; }

private:
	arch::core *owning_core_;

protected:
	__aligned(16) tcb tcb_;
};
} // namespace stacsos::kernel::sched
