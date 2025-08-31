/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::storage {
enum class block_io_request_direction { read, write };

struct block_io_request;
typedef void (*io_request_cb)(block_io_request *, void *);

struct block_io_request {
	block_io_request_direction direction;
	u64 start_block;
	u64 block_count;
	void *buffer;
	io_request_cb callback;
	void *cb_state;
};

class block_device : public device {
public:
	static device_class block_device_class;

	block_device(device_class &devclass, bus &parent)
		: device(devclass, parent)
	{
	}

	virtual ~block_device() { }

	virtual u64 nr_blocks() const = 0;

	virtual void submit_io_request(block_io_request &request) = 0;

	void read_blocks_sync(void *buffer, u64 start, u64 count);
	void write_blocks_sync(const void *buffer, u64 start, u64 count);

private:
	void submit_sync_request(block_io_request_direction direction, void *buffer, u64 start, u64 count);
};
} // namespace stacsos::kernel::dev::storage
