/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::dev::input {
enum class keys {
	NO_KEY = 0,
	UNKNOWN_KEY,

	KEY_ESCAPE,

	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,

	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,

	KEY_NUMPAD_1,
	KEY_NUMPAD_2,
	KEY_NUMPAD_3,
	KEY_NUMPAD_4,
	KEY_NUMPAD_5,
	KEY_NUMPAD_6,
	KEY_NUMPAD_7,
	KEY_NUMPAD_8,
	KEY_NUMPAD_9,
	KEY_NUMPAD_0,

	KEY_NUMPAD_ADD,
	KEY_NUMPAD_SUBTRACT,
	KEY_NUMPAD_MULTIPLY,
	KEY_NUMPAD_DIVIDE,
	KEY_NUMPAD_COMMA,

	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEY_LSQBRACKET,
	KEY_RSQBRACKET,

	KEY_DOT,
	KEY_COMMA,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_HASH,
	KEY_BACKSLASH,
	KEY_FORSLASH,
	KEY_BACKTICK,
	KEY_HYPHEN,
	KEY_EQUALS,

	KEY_TAB,
	KEY_RETURN,
	KEY_SPACE,
	KEY_BACKSPACE,

	KEY_PRINT,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_SCROLLOCK,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LCTRL,
	KEY_RCTRL,
	KEY_LALT,
	KEY_RALT,
	KEY_LWIN,
	KEY_RWIN,
};
}
