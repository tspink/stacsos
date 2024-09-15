/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/console-interface.h>
#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::console {
class virtual_console;
}

namespace stacsos::kernel::dev::tty {
class terminal : public device, public arch::console_interface {
public:
	static device_class terminal_device_class;
	terminal(bus &owner)
		: device(terminal_device_class, owner)
		, current_attr_(0x07)
	{
	}

	virtual void configure() override;

	virtual void write_char(unsigned char ch, u8 attr) override;

	void write(const void *buffer, size_t size);
	void read(void *buffer, size_t size);

	void attach(console::virtual_console &vc) { attached_vc_ = &vc; }

	shared_ptr<fs::file> open_as_file() override;

private:
	console::virtual_console *attached_vc_;
	int current_attr_;
};
} // namespace stacsos::kernel::dev::tty
