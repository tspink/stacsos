/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/bus.h>
#include <stacsos/kernel/dev/device-class.h>
#include <stacsos/kernel/fs/file.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::fs {
class file;
}

namespace stacsos::kernel::dev {

class device {
public:
	device(device_class &devclass, bus &bus)
		: devclass_(devclass)
		, bus_(bus)
	{
	}

	virtual ~device() { }
	device_class &devclass() const { return devclass_; }

	bus &parent_bus() const { return bus_; }

	virtual void configure() = 0;

	virtual shared_ptr<fs::file> open_as_file() { return nullptr; }

private:
	device_class &devclass_;
	bus &bus_;
};
} // namespace stacsos::kernel::dev
