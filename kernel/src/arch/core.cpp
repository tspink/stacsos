/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/arch/timer.h>
#include <stacsos/kernel/arch/x86/cregs.h>
#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-allocator.h>
#include <stacsos/kernel/sched/schedulable-entity.h>

using namespace stacsos::kernel::arch;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::mem;
using namespace stacsos::kernel;

core &core::this_core()
{
	auto &c = core_manager::get().get_core(core::this_core_id());
	assert(&c != nullptr);

	return c;
}

static void idle_thread()
{
	while (true) {
		__relax();
	}
}

extern "C" __noreturn void x86_return_to_task();

void core::run()
{
	// This routine MUST be called from the same core, i.e. the core that is executing must call the run for
	// the core object that represents it.
	if (core::this_core_id() != id()) {
		panic("run must be called on executing core");
	}

	// Initialise the IDLE thread, for doing nothing when there are no tasks to run.
	void *idle_thread_stack = memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero)->base_address_ptr();

	idle_thread_.mcontext = (machine_context *)idle_thread_stack;
	idle_thread_.mcontext->cs = KERNEL_CODE_SEGMENT_SELECTOR;
	idle_thread_.mcontext->ss = KERNEL_DATA_SEGMENT_SELECTOR;
	idle_thread_.mcontext->rflags = 0x202; // RSVD | IF
	idle_thread_.mcontext->rip = (u64)idle_thread;
	idle_thread_.mcontext->rsp = (u64)idle_thread_stack + PAGE_SIZE;
	idle_thread_.mcontext->gs = (u64)&idle_thread_;
	idle_thread_.cr3 = memory_manager::get().root_address_space().pgtable().effective_cr3();
	idle_thread_.kernel_stack = (u64)idle_thread_stack + PAGE_SIZE;

	set_current_tcb(&idle_thread_);

	dprintf("core [%d]: run\n", id());
	local_timer().start(100); // 100 Hz

	// This will also enable interrupts, because the IF flag is set in rflags.
	x86_return_to_task();
	__unreachable();
}

void core::schedule()
{
	tcb *next = sched_alg_->select_next_task(get_current_tcb());
	if (!next) {
		next = &idle_thread_;
	}

	set_current_tcb(next);
}

void core::update_accounting()
{
	// A thread has just been interrupted by the timer

	tcb *current = get_current_tcb();
	if (current) {
		u64 now = __builtin_ia32_rdtsc();
		u64 delta = now - current->start_time;
		current->run_time += delta;
		current->start_time = now;
	}
}
