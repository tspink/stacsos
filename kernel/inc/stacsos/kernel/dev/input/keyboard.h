/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>
#include <stacsos/kernel/dev/input/keys.h>
#include <stacsos/list.h>

namespace stacsos::kernel::arch {
class irq;
}

namespace stacsos::kernel::dev::input {

class keyboard_listener {
public:
	virtual void on_key_up(keys key) = 0;
	virtual void on_key_down(keys key) = 0;
};

enum class key_event_state { normal, e0, e1 };
class keyboard : public device {
public:
	static device_class keyboard_device_class;

	keyboard(bus &parent)
		: device(keyboard_device_class, parent)
		, irq_(nullptr)
		, listener_(nullptr)
		, kes_(key_event_state::normal)
	{
	}

	virtual void configure() override;

	void set_listener(keyboard_listener &l) { listener_ = &l; }

private:
	arch::irq *irq_;
	keyboard_listener *listener_;
	key_event_state kes_;

	static void keyboard_irq_handler(u8, void *, void *);
	void handle_key_event(u8 data);
	keys scancode_to_key(u8 scancode);
};
} // namespace stacsos::kernel::dev::input
