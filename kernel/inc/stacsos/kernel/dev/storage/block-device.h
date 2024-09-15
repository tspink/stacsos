/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::storage {
class block_device : public device {
public:
	static device_class block_device_class;

	block_device(device_class &devclass, bus &parent)
		: device(devclass, parent)
	{
	}

	virtual ~block_device() { }

	virtual u64 nr_blocks() const = 0;

	virtual void read_blocks_sync(void *buffer, u64 start, u64 count) = 0;
	virtual void write_blocks_sync(const void *buffer, u64 start, u64 count) = 0;
};
} // namespace stacsos::kernel::dev::storage
