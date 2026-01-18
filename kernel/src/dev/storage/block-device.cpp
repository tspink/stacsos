/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/block-device.h>
#include <stacsos/kernel/sched/completion.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;
using namespace stacsos::kernel::sched;

device_class block_device::block_device_class(device_class::root, "blk");

void block_device::submit_io_request(block_io_request &request) { submit_real_io_request(request); }

void block_device::read_blocks_sync(void *buffer, u64 start, u64 count) { submit_sync_request(block_io_request_direction::read, buffer, start, count); }

void block_device::write_blocks_sync(const void *buffer, u64 start, u64 count)
{
	submit_sync_request(block_io_request_direction::write, (void *)buffer, start, count);
}

void block_device::submit_sync_request(block_io_request_direction direction, void *buffer, u64 start, u64 count)
{
	block_io_request io_req;
	io_req.direction = direction;
	io_req.start_block = start;
	io_req.block_count = count;
	io_req.buffer = buffer;

	submit_io_request(io_req);

	io_req.completion.wait();
}
