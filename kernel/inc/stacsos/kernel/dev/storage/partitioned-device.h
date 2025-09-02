/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/block-device.h>

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

protected:
	virtual void submit_real_io_request(block_io_request &request) override
	{
		block_io_request *underlying_request = new block_io_request();
		underlying_request->direction = request.direction;
		underlying_request->block_count = request.block_count;
		underlying_request->start_block = request.start_block + block_offset_;
		underlying_request->buffer = request.buffer;
		underlying_request->callback = partition_request_cb;

		callback_state *cb_state = new callback_state();
		cb_state->original_request = &request;

		underlying_request->cb_state = cb_state;

		owner_.submit_io_request(*underlying_request);
	}

private:
	struct callback_state {
		block_io_request *original_request;
	};

	static void partition_request_cb(block_io_request *request, void *state)
	{
		callback_state *cb_state = (callback_state *)state;

		if (cb_state->original_request->callback) {
			cb_state->original_request->callback(cb_state->original_request, cb_state->original_request->cb_state);
		}

		delete cb_state;
		delete request;
	}

	block_device &owner_;
	u64 block_offset_;
	u64 nr_blocks_;
};
} // namespace stacsos::kernel::dev::storage
