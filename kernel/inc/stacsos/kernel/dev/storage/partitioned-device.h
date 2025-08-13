/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/storage/block-device.h>
#include <stacsos/kernel/debug.h>

namespace stacsos::kernel::dev::storage {
class partition;
class partitioned_device {
	friend class partition;

public:
	partitioned_device(block_device &parent)
		: parent_(parent)
	{
	}

	virtual ~partitioned_device() { }

	virtual void scan() = 0;

protected:
	block_device &parent() const { return parent_; }

private:
	block_device &parent_;
};

class partition : public block_device {
public:
	static device_class partition_device_class;

	partition(block_device &underlying_block_device, u64 block_offset, u64 nr_blocks)
		: block_device(partition_device_class, underlying_block_device.parent_bus())
		, owner_(underlying_block_device)
		, block_offset_(block_offset)
		, nr_blocks_(nr_blocks)
	{
	}

	virtual ~partition() { }

	virtual void configure() override { }

	virtual u64 nr_blocks() const override { return nr_blocks_; }

	virtual void read_blocks_sync(void *buffer, u64 start, u64 count) override
	{
		owner_.read_blocks_sync(buffer, start + block_offset_, count);
	}

	virtual void write_blocks_sync(const void *buffer, u64 start, u64 count) override
	{
		owner_.write_blocks_sync(buffer, start + block_offset_, count);
	}

private:
	block_device &owner_;
	u64 block_offset_;
	u64 nr_blocks_;
};
} // namespace stacsos::kernel::dev::storage
