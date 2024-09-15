/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::dev {
class device_manager;
class device;

class device_class {
	friend class device_manager;
	friend class device;

public:
	static device_class root;

	device_class(device_class &parent, const char *name)
		: parent_(&parent)
		, name_(name)
		, index_(0)
	{
	}

	const char *name() const { return name_; }
	device_class &parent() const { return *parent_; }

	bool is_a(const device_class &dc)
	{
		if (&dc == this)
			return true;
		if (parent_ == nullptr)
			return false;
		return parent_->is_a(dc);
	}

	u64 get_next_index() { return index_++; }

private:
	explicit device_class()
		: parent_(nullptr)
		, name_("root")
		, index_(0)
	{
	}

	device_class *parent_;
	const char *name_;
	u64 index_;
};
} // namespace stacsos::kernel::dev
