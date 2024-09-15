/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/helpers.h>

typedef unsigned int spinlock_var_t;
extern "C" void spinlock_acquire(spinlock_var_t *lv);
extern "C" void spinlock_release(spinlock_var_t *lv);
extern "C" void spinlock_irq_acquire(spinlock_var_t *lv, u64 *flags);
extern "C" void spinlock_irq_release(spinlock_var_t *lv, u64 flags);

namespace stacsos::kernel {
class spinlock {
public:
	spinlock()
		: spin_lock_var_(0)
	{
	}

	void lock() { ::spinlock_acquire(&spin_lock_var_); }
	void unlock() { ::spinlock_release(&spin_lock_var_); }

private:
	DELETE_DEFAULT_COPY_AND_MOVE(spinlock);

	spinlock_var_t spin_lock_var_;
};

class spinlock_irq {
public:
	spinlock_irq()
		: spin_lock_var_(0)
	{
	}

	void lock(u64 *flags) { ::spinlock_irq_acquire(&spin_lock_var_, flags); }
	void unlock(u64 flags) { ::spinlock_irq_release(&spin_lock_var_, flags); }

private:
	DELETE_DEFAULT_COPY_AND_MOVE(spinlock_irq);

	spinlock_var_t spin_lock_var_;
};

class unique_irq_lock {
public:
	explicit unique_irq_lock(spinlock_irq &o)
		: mutex_(&o)
		, owner_(false)
	{
		lock();
		owner_ = true;
	}

	~unique_irq_lock() { unlock(); }

	unique_irq_lock(const unique_irq_lock &) = delete;
	unique_irq_lock &operator=(const unique_irq_lock &) = delete;

	unique_irq_lock(unique_irq_lock &&o) noexcept
		: mutex_(o.mutex_)
		, owner_(o.owner_)
	{
		o.mutex_ = nullptr;
		o.owner_ = false;
	}

	unique_irq_lock &operator=(unique_irq_lock &&o) noexcept
	{
		if (owner_)
			unlock();

		unique_irq_lock(move(o)).swap(*this);

		o.mutex_ = nullptr;
		o.owner_ = false;

		return *this;
	}

	void lock()
	{
		if (mutex_) {
			mutex_->lock(&flags_);
			owner_ = true;
		}
	}

	void unlock()
	{
		if (mutex_ && owner_) {
			mutex_->unlock(flags_);
			owner_ = false;
		}
	}

	void swap(unique_irq_lock &o) noexcept
	{
		stacsos::swap(mutex_, o.mutex_);
		stacsos::swap(owner_, o.owner_);
	}

	spinlock_irq *release() noexcept
	{
		spinlock_irq *r = mutex_;
		mutex_ = nullptr;
		owner_ = false;

		return r;
	}

	bool is_owner() const noexcept { return owner_; }

	explicit operator bool() const noexcept { return is_owner(); }

	spinlock_irq *mutex() const noexcept { return mutex_; }

private:
	spinlock_irq *mutex_;
	bool owner_;
	u64 flags_;
};

} // namespace stacsos::kernel
