/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/helpers.h>
#include <stacsos/kernel/mem/page.h>

namespace stacsos::kernel::mem {
class page_alloc_ref {
public:
	page_alloc_ref()
		: page_(nullptr)
		, order_(-1)
	{
	}

	page_alloc_ref(page *page, int order)
		: page_(page)
		, order_(order)
	{
		acquire();
	}

	page_alloc_ref(const page_alloc_ref &o)
		: page_(o.page_)
		, order_(o.order_)
	{
		acquire();
	}

	page_alloc_ref(page_alloc_ref &&o)
		: page_(o.page_)
		, order_(o.order_)
	{
	}

	~page_alloc_ref() { release(); }

	page_alloc_ref &operator=(page_alloc_ref o)
	{
		release();
		swap(*this, o);
		acquire();

		return *this;
	}

	friend void swap(page_alloc_ref &a, page_alloc_ref &b)
	{
		using stacsos::swap;

		swap(a.page_, b.page_);
		swap(a.order_, b.order_);
	}

	page &operator*() const { return *page_; }
	page *operator->() const { return page_; }

	page *get() const { return page_; }

	u64 refcount() const { return page_ ? page_->refcount() : 0; }

private:
	page *page_;
	int order_;

	void acquire()
	{
		if (page_) {
			// udprintf("PAGE %p ACQUIRE %u\n", page_->ptr(), refcount());
			page_->acquire();
		}
	}

	void release()
	{
		if (page_) {
			// udprintf("PAGE %p RELEASE %u\n", page_->ptr(), refcount());
			if (page_->release()) {
				free();
			}
		}
	}

	void free()
	{
		// udprintf("PAGE %p FREE\n", page_->ptr());
	}
};
} // namespace stacsos::kernel::mem
