/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>
#include <stacsos/kernel/dev/input/keyboard.h>
#include <stacsos/kernel/dev/input/keys.h>
#include <stacsos/list.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::dev::gfx {
class graphics;
}

namespace stacsos::kernel::dev::console {
class virtual_console;

class physical_console : public device, public input::keyboard_listener {
public:
	static device_class physical_console_device_class;

	physical_console(bus &owner, gfx::graphics &gdev)
		: device(physical_console_device_class, owner)
		, current_vc_(nullptr)
		, saved_vc_buffer_(nullptr)
		, saved_vc_buffer_size_(0)
		, gdev_(gdev)
		, alt_pressed_(false)
	{
	}

	virtual ~physical_console() { }

	virtual void configure() override;

	void add_virtual_console(virtual_console &vc)
	{
		virtual_consoles_.append(&vc);
		if (!current_vc_) {
			set_current_vc(vc);
		}
	}

	void set_current_vc(virtual_console &vc)
	{
		virtual_console *prev_console = current_vc_;
		current_vc_ = &vc;

		on_vc_changed(prev_console, current_vc_);
	}

	virtual_console &get_current_vc() const { return *current_vc_; }

	virtual void on_key_up(input::keys key) override;
	virtual void on_key_down(input::keys key) override;

private:
	list<virtual_console *> virtual_consoles_;
	virtual_console *current_vc_;

	u8 *saved_vc_buffer_;
	size_t saved_vc_buffer_size_;

	gfx::graphics &gdev_;
	bool alt_pressed_;

	void on_vc_changed(virtual_console *prev, virtual_console *next);
};
} // namespace stacsos::kernel::dev::console
