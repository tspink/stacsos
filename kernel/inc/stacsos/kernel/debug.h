/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel {
namespace arch {
	class console_interface;
}

extern void dprintf_init();
extern void dprintf_set_console(arch::console_interface *iface);
extern void dprintf(const char *msg, ...);

class debug_helper {
	DEFINE_SINGLETON(debug_helper);

private:
	debug_helper() { }

public:
	void parse_image();
};
} // namespace stacsos::kernel
