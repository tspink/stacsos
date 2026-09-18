#pragma once

#include <stacsos/kernel/lock.h>
#include <stacsos/kernel/sched/thread.h>

namespace stacsos::kernel::sched {
class completion {
public:
	completion()
		: signalled_(false)
		, waiter_(nullptr)
	{
	}

	void wait()
	{
		u64 flags;
		lock_.lock(&flags);

		if (waiter_) {
			panic("only one waiter allowed");
		}

		waiter_ = &thread::current();

		while (!signalled_) {
			waiter_->suspend();
			lock_.unlock(flags);

			asm volatile("int $0x81");

			lock_.lock(&flags);
		}

		lock_.unlock(flags);
	}

	void signal()
	{
		unique_irq_lock l(lock_);

		signalled_ = true;
		if (waiter_) {
			waiter_->resume();
			waiter_ = nullptr;
		}
	}

private:
	spinlock_irq lock_;
	bool signalled_;
	thread *waiter_;
};
} // namespace stacsos::kernel::sched
