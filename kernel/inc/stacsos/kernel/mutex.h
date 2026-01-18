#pragma once

#include <stacsos/kernel/lock.h>
#include <stacsos/kernel/sched/thread.h>
#include <stacsos/list.h>

namespace stacsos::kernel {
class mutex {
public:
	mutex()
		: owner_(nullptr)
	{
	}

	void lock()
	{
		sched::thread *this_thread = &sched::thread::current();

		//__sync_val_compare_and_swap(ptr, oldval, newval)

	retry:
		if (__sync_val_compare_and_swap(&owner_, nullptr, this_thread) == nullptr) {
			// lock acquired
		} else {
			u64 flags;

			// lock contended
			wait_list_lock_.lock(&flags);

			if (__sync_val_compare_and_swap(&owner_, nullptr, this_thread) == nullptr) {
				// lock acquired
				wait_list_lock_.unlock(flags);
				return;
			}

			// Ok, we have to go to sleep.

			// (1) Add ourselves to the waiters list.
			waiters_.append(this_thread);

			// (2) Set thread state to suspended.
			this_thread->suspend();

			// (3) Release the wait list lock
			wait_list_lock_.unlock(flags);

			// (4) Yield to the scheduler
			asm volatile("int $0xff");

			// Why does this work?
			// We're holding an IRQ lock at (1), so we can't be pre-empted
			// So, we can add ourselves to the waiters list
			// Then, we set ourselves to suspended -- but don't actually stop executing
			// We're still running with interrupts disabled.
			// Then we release the wait list lock, which re-enables interrupts
			// Then, we yield to the scheduler.
			//   Either, 1: we were pre-empted before we get to the yield, which is fine, because we're
			//   technically suspended.  If we get woken up, then when we return, we'll yield again,
			//   which is a shame, but not incorrect.
			//   2: we reach the yield statement, and give up the CPU, thus being de-scheduled because our
			//   state is suspended.
			//
			// When we are woken up, we try again...

			goto retry;
		}
	}

	void unlock()
	{
		sched::thread *this_thread = &sched::thread::current();

		if (__sync_val_compare_and_swap(&owner_, this_thread, nullptr) != this_thread) {
			panic("attempted to release lock we don't own");
		}

		// wake up a thread
		u64 flags;
		wait_list_lock_.lock(&flags);
		if (!waiters_.empty()) {
			auto waiter = waiters_.pop();
			waiter->resume();
		}

		wait_list_lock_.unlock(flags);
	}

	bool is_locked_by_me() { return owner_ == &sched::thread::current(); }

private:
	sched::thread *owner_;
	spinlock_irq wait_list_lock_;
	list<sched::thread *> waiters_;
};
} // namespace stacsos::kernel
