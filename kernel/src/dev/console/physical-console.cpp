/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/console/physical-console.h>
#include <stacsos/kernel/dev/console/virtual-console.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/gfx/graphics.h>
#include <stacsos/kernel/dev/input/keyboard.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::console;
using namespace stacsos::kernel::dev::input;

device_class physical_console::physical_console_device_class(device_class::root, "physcon");

void physical_console::configure()
{
	auto &kbd = device_manager::get().get_device_by_class<keyboard>(keyboard::keyboard_device_class);
	kbd.set_listener(*this);
}

void physical_console::on_vc_changed(virtual_console *prev_vc, virtual_console *next_vc)
{
	if (prev_vc) {
		auto tmp = prev_vc->internal_buffer_;
		prev_vc->internal_buffer_ = saved_vc_buffer_;
		memops::memcpy(prev_vc->internal_buffer_, tmp, saved_vc_buffer_size_);

		prev_vc->deactivate();
	}

	assert(next_vc);

	saved_vc_buffer_ = next_vc->internal_buffer_;
	saved_vc_buffer_size_ = next_vc->internal_buffer_size_;

	switch (next_vc->mode()) {
	case virtual_console_mode::text:
		gdev_.reset();
		next_vc->internal_buffer_ = (u8 *)(0xffff'8000'000b'8000ull);
		break;

	case virtual_console_mode::gfx:
		gdev_.set_mode(640, 480, 32);
		next_vc->internal_buffer_ = (u8 *)gdev_.fb();
		break;
	}

	memops::memcpy(next_vc->internal_buffer_, saved_vc_buffer_, saved_vc_buffer_size_);
	next_vc->activate();
}

void physical_console::on_key_down(keys key)
{
	switch (key) {
	case keys::KEY_F1:
		if (alt_pressed_) {
			set_current_vc(*virtual_consoles_.at(0));
			return;
		}
		break;

	case keys::KEY_F2:
		if (alt_pressed_ && virtual_consoles_.count() > 1) {
			set_current_vc(*virtual_consoles_.at(1));
			return;
		}
		break;

	case keys::KEY_F3:
		if (alt_pressed_ && virtual_consoles_.count() > 2) {
			set_current_vc(*virtual_consoles_.at(2));
			return;
		}
		break;

	case keys::KEY_F4:
		if (alt_pressed_ && virtual_consoles_.count() > 3) {
			set_current_vc(*virtual_consoles_.at(3));
			return;
		}
		break;

	case keys::KEY_LALT:
	case keys::KEY_RALT:
		alt_pressed_ = true;
		break;

	default:
		break;
	}

	if (current_vc_) {
		current_vc_->on_key_down(key);
	}
}

void physical_console::on_key_up(keys key)
{
	switch (key) {
	case keys::KEY_LALT:
	case keys::KEY_RALT:
		alt_pressed_ = false; // Fallthrough

	default:
		if (current_vc_) {
			current_vc_->on_key_up(key);
		}
		break;
	}
}
