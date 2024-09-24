/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page.h>
#include <stacsos/kernel/sched/process.h>
#include <stacsos/kernel/sched/scheduler.h>
#include <stacsos/kernel/sched/thread.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::mem;
using stacsos::kernel::arch::x86::machine_context;

thread::thread(process &owner, u64 ep, void *ep_arg, u64 user_stack)
	: owner_(owner)
	, ep_(ep)
	, arg_(ep_arg)
	, state_(thread_states::created)
	, kernel_stack_(nullptr)
	, user_stack_(user_stack)
{
	init_tcb();
	change_state(thread_states::created);
}

thread &thread::current()
{
	auto current_tcb = stacsos::kernel::arch::core::this_core().get_current_tcb();
	assert(current_tcb != nullptr);

	return *(thread *)(current_tcb->entity);
}

bool thread::is_self() const { return &thread::current() == this; }

void thread::start() { change_state(thread_states::runnable); }
void thread::stop()
{
	change_state(thread_states::terminated);
	owner_.on_thread_stopped(*this);
}
void thread::suspend() { change_state(thread_states::suspended); }
void thread::resume() { change_state(thread_states::runnable); }

void thread::task_entry_trampoline(thread *thread)
{
	// If there is an entry point, then run it and stop the task once it has completed.
	if (thread->ep_) {
		auto epfn = (void (*)(void *))thread->ep_;
		epfn(thread->arg_);
		thread->stop();
	}

	// If there is no entry point (i.e. this is an IDLE task), or the task has signalled
	// to stop, just hang in a loop waiting to be terminated.
	hang_loop();
}

void thread::init_tcb()
{
	// Allocate an 8k stack.
	kernel_stack_ = memory_manager::get().pgalloc().allocate_pages(stack_size_order, page_allocation_flags::zero);

	// Set the pointer to the task object in the task control block, and pop the initial
	// machine context into the stack.
	tcb_.entity = this;
	tcb_.mcontext = (machine_context *)(((uintptr_t)kernel_stack_->base_address_ptr() + stack_size) - sizeof(machine_context));
	tcb_.cr3 = owner_.addrspace().pgtable().effective_cr3();
	tcb_.kernel_stack = (u64)kernel_stack_->base_address_ptr() + stack_size;
	tcb_.user_stack_save = 0;

	// Fill in the required values for starting this task in the initial context.

	if (owner_.privilege() == exec_privilege::kernel) {
		tcb_.mcontext->cs = KERNEL_CODE_SEGMENT_SELECTOR;
		tcb_.mcontext->ss = KERNEL_DATA_SEGMENT_SELECTOR;
	} else {
		tcb_.mcontext->cs = USER_CODE_SEGMENT_SELECTOR | 3;
		tcb_.mcontext->ss = USER_DATA_SEGMENT_SELECTOR | 3;
	}

	tcb_.mcontext->rflags = 0x202; // RSVD | IF

	if (owner_.privilege() == exec_privilege::kernel) {
		tcb_.mcontext->rip = (u64)task_entry_trampoline; // The actual entry point for a kernel task is the
														 // trampoline.  This is so that tasks entry points can return, and
														 // they won't return into "nothing", instead we'll take control back
														 // and stop the task.

		tcb_.mcontext->rdi = (u64)this; // The first argument to the trampoline is a pointer to this task object.

		// The stack pointer needs to point to the allocated stack.
		tcb_.mcontext->rsp = (u64)((uintptr_t)kernel_stack_->base_address_ptr() + stack_size);

		// The GS register needs to point to the TCB, so that the kernel thread can manipulate itself.
		tcb_.mcontext->gs = (u64)&tcb_;
	} else {
		tcb_.mcontext->rip = ep_;
		tcb_.mcontext->rdi = (u64)arg_;
		tcb_.mcontext->rsp = user_stack_;
	}
}

void thread::change_state(thread_states new_state)
{
	// Ignore threads whose state isn't actually changing (unless the state
	// is "created")
	if (state_ == new_state && state_ != thread_states::created) {
		return;
	}

	switch (new_state) {
	case thread_states::created: // thread is newly created
		switch (state_) {
		case thread_states::created:
			state_ = new_state;
			break;

		default:
			panic("illegal thread state change");
		}
		break;

	case thread_states::runnable: // thread is becoming runnable
		switch (state_) {
		case thread_states::created:
		case thread_states::running:
		case thread_states::suspended:
			state_ = new_state;
			scheduler::get().add_to_schedule(*this);
			break;

		default:
			panic("illegal thread state change");
		}
		break;

	case thread_states::running: // thread is running on a core
		switch (state_) {
		case thread_states::runnable:
			state_ = new_state;
			break;

		default:
			panic("illegal thread state change");
		}
		break;

	case thread_states::suspended: // thread is going to sleep
		switch (state_) {
		case thread_states::runnable:
		case thread_states::running:
			state_ = new_state;
			scheduler::get().remove_from_schedule(*this);
			break;

		default:
			panic("illegal thread state change");
		}
		break;

	case thread_states::terminated: // thread has been terminated
		switch (state_) {
		case thread_states::runnable:
		case thread_states::running:
		case thread_states::suspended:
			state_ = new_state;
			scheduler::get().remove_from_schedule(*this);
			break;

		default:
			panic("illegal thread state change");
		}
		break;

	default:
		panic("illegal thread state change");
	}

	state_changed_event_.trigger();
}
