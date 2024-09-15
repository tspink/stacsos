/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/arch/x86/ioapic.h>
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/arch/x86/x2apic.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/arch/x86/x86-platform.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/input/keyboard.h>

using namespace stacsos::kernel::arch;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::input;

device_class keyboard::keyboard_device_class(device_class::root, "kbd");

void keyboard::configure()
{
	// irq_ = core_manager::get().get_boot_core().irqs().request_irq();
	// irq_->attach(keyboard_irq_handler, this);

	// we've requested an irq from the boot core, to handle the keyboard interrupt
	// now we need to map it to the physical irq
	x86_platform::get().get_ioapic()->allocate_physical_irq(1, (x86_core &)core_manager::get().get_boot_core(), keyboard_irq_handler, this);
}

void keyboard::keyboard_irq_handler(u8 irq, void *ctx, void *arg)
{
	keyboard *device = (keyboard *)arg;

	u8 key_event_data = ioports::keyboard_controller::read8();

	device->handle_key_event(key_event_data);

	((x86_core &)core::this_core()).lapic().eoi();
}

void keyboard::handle_key_event(u8 key_event_data)
{
	if (listener_) {
		u8 scancode = key_event_data & ~0x80;

		switch (kes_) {
		case key_event_state::normal: {
			if (scancode == 0xe0) {
				kes_ = key_event_state::e0;
				return;
			}

			keys key = scancode_to_key(scancode);

			if (key_event_data & 0x80) {
				listener_->on_key_up(key);
			} else {
				listener_->on_key_down(key);
			}
		} break;

		case key_event_state::e0:
			kes_ = key_event_state::normal;
			break;

		case key_event_state::e1:
			kes_ = key_event_state::normal;
			break;
		}
	}
}

static keys scancode_map[] = {
	keys::NO_KEY, //
	keys::KEY_ESCAPE, //
	keys::KEY_1, //
	keys::KEY_2, //
	keys::KEY_3, //
	keys::KEY_4, //
	keys::KEY_5, //
	keys::KEY_6, //
	keys::KEY_7, //
	keys::KEY_8, //
	keys::KEY_9, //
	keys::KEY_0, //
	//
	keys::KEY_HYPHEN, //
	keys::KEY_EQUALS, //
	keys::KEY_BACKSPACE, //
	keys::KEY_TAB, //
	//
	keys::KEY_Q, //
	keys::KEY_W, //
	keys::KEY_E, //
	keys::KEY_R, //
	keys::KEY_T, //
	keys::KEY_Y, //
	keys::KEY_U, //
	keys::KEY_I, //
	keys::KEY_O, //
	keys::KEY_P, //
	keys::KEY_LSQBRACKET, //
	keys::KEY_RSQBRACKET, //
	//
	keys::KEY_RETURN, //
	keys::KEY_LCTRL, //
	//
	keys::KEY_A, ////
	keys::KEY_S, //
	keys::KEY_D, //
	keys::KEY_F, //
	keys::KEY_G, //
	keys::KEY_H, //
	keys::KEY_J, //
	keys::KEY_K, //
	keys::KEY_L, //
	keys::KEY_SEMICOLON, //
	keys::KEY_APOSTROPHE, //
	keys::KEY_BACKTICK, //
	//
	keys::KEY_LSHIFT, //
	keys::KEY_HASH, //
	keys::KEY_Z, //
	keys::KEY_X, //
	keys::KEY_C, //
	keys::KEY_V, //
	keys::KEY_B, //
	keys::KEY_N, //
	keys::KEY_M, //
	keys::KEY_COMMA, //
	keys::KEY_DOT, //
	keys::KEY_FORSLASH, //
	//
	keys::KEY_RSHIFT, //
	keys::KEY_NUMPAD_MULTIPLY, //
	keys::KEY_LALT, //
	keys::KEY_SPACE, //
	keys::KEY_CAPSLOCK, //
	//
	keys::KEY_F1, //
	keys::KEY_F2, //
	keys::KEY_F3, //
	keys::KEY_F4, //
	keys::KEY_F5, //
	keys::KEY_F6, //
	keys::KEY_F7, //
	keys::KEY_F8, //
	keys::KEY_F9, //
	keys::KEY_F10, //
	//
	keys::KEY_NUMLOCK, //
	keys::KEY_SCROLLOCK, //
	keys::KEY_NUMPAD_7, //
	keys::KEY_NUMPAD_8, //
	keys::KEY_NUMPAD_9, //
	keys::KEY_NUMPAD_SUBTRACT, //
	keys::KEY_NUMPAD_4, //
	keys::KEY_NUMPAD_5, //
	keys::KEY_NUMPAD_6, //
	keys::KEY_NUMPAD_ADD, //
	keys::KEY_NUMPAD_1, //
	keys::KEY_NUMPAD_2, //
	keys::KEY_NUMPAD_3, //
	keys::KEY_NUMPAD_0, //
	keys::KEY_NUMPAD_COMMA, //
	//
	keys::KEY_PRINT, //
	keys::KEY_0, // NONE
	keys::KEY_BACKSLASH, //
	//
	keys::KEY_F11, //
	keys::KEY_F12 //
};

keys keyboard::scancode_to_key(u8 scancode) { return scancode < sizeof(scancode_map) / sizeof(scancode_map[0]) ? scancode_map[scancode] : keys::NO_KEY; }
