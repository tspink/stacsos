/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/helpers.h>

namespace stacsos {
template <class T> class unique_ptr {
public:
	explicit unique_ptr(T *ptr)
		: ptr_(ptr)
	{
		// dprintf("acq %p\n", ptr);
	}

	template <class U>
	explicit unique_ptr(U *ptr)
		: ptr_((T *)ptr)
	{
		// dprintf("acq %p\n", ptr);
	}

	~unique_ptr()
	{
		// dprintf("del %p\n", ptr_);
		delete ptr_;
	}

	unique_ptr(unique_ptr const &) = delete;
	unique_ptr &operator=(unique_ptr const &) = delete;

	unique_ptr(unique_ptr &&o) noexcept
		: ptr_(o.ptr_)
	{
		o.ptr_ = nullptr;
	}

	unique_ptr &operator=(unique_ptr &&o)
	{
		ptr_ = o.ptr_;
		o.ptr_ = nullptr;

		return *this;
	}

	T *operator->() const { return ptr_; }
	T &operator*() const { return *ptr_; }

	T *get() const { return ptr_; }
	explicit operator bool() const { return ptr_; }

	T *release()
	{
		T *result = nullptr;
		swap(result, ptr_);

		return result;
	}

private:
	T *ptr_;
};

template <class T, class... U> unique_ptr<T> make_unique(U &&...u) { return unique_ptr<T>(new T(forward<U>(u)...)); }

template <class T> class shared_ptr {
public:
	using element_type = T;

	explicit shared_ptr()
		: ptr_(nullptr)
		, refcount_(nullptr)
	{
	}

	shared_ptr(decltype(nullptr))
		: ptr_(nullptr)
		, refcount_(nullptr)
	{
	}

	// Construct from pointer
	explicit shared_ptr(T *ptr)
		: ptr_(ptr)
		, refcount_(nullptr)
	{
		if (ptr) {
			acquire();
		}
	}

	// Construct from casted pointer
	template <class U>
	explicit shared_ptr(U *ptr)
		: ptr_((T *)ptr)
		, refcount_(nullptr)
	{
		if (ptr) {
			acquire();
		}
	}

	// Destroy
	~shared_ptr() { release(); }

	// Copy constructor (with cast)
	template <class U>
	shared_ptr(const shared_ptr<U> &other)
		: ptr_(other.ptr_)
		, refcount_(other.refcount_)
	{
		acquire();
	}

	// Copy constructor
	shared_ptr(const shared_ptr<T> &other)
		: ptr_(other.ptr_)
		, refcount_(other.refcount_)
	{
		acquire();
	}

	// Move constructor
	shared_ptr(shared_ptr<T> &&other) noexcept
		: shared_ptr()
	{
		swap(*this, other);
	}

	shared_ptr<T> &operator=(shared_ptr<T> other)
	{
		swap(*this, other);

		acquire();
		return *this;
	}

	shared_ptr<T> &operator=(shared_ptr<T> &&other)
	{
		swap(*this, other);
		return *this;
	}

	operator bool() const { return use_count() > 0; }
	bool unique() const { return use_count() == 1; }
	u64 use_count() const { return refcount_ == nullptr ? 0 : *refcount_; }

	T &operator*() { return *ptr_; }

	T *operator->() const
	{
		if (!ptr_)
			panic("dereferencing null pointer");
		return ptr_;
	}

	T *get(void) const { return ptr_; }

	friend void swap(shared_ptr &a, shared_ptr &b) noexcept
	{
		swap(a.ptr_, b.ptr_);
		swap(a.refcount_, b.refcount_);
	}

private:
	void acquire()
	{
		if (refcount_ == nullptr) {
			refcount_ = new u64(1);
		} else {
			(*refcount_)++;
		}
	}

	void release()
	{
		if (refcount_ != nullptr) {
			(*refcount_)--;
			if (*refcount_ == 0) {
				if (ptr_ != nullptr) {
					(void)sizeof(T);
					delete ptr_;
					ptr_ = nullptr;
				}

				delete refcount_;
				refcount_ = nullptr;
			}
		}
	}

	template <class U> friend class shared_ptr;

	T *ptr_;
	u64 *refcount_;
};

template <class T, class... U> shared_ptr<T> make_shared(U &&...u) { return shared_ptr<T>(new T(forward<U>(u)...)); }
} // namespace stacsos
