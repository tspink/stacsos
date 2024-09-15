/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::gfx {
class graphics : public device {
public:
	static device_class graphics_device_class;

	graphics(device_class &dc, bus &owner)
		: device(dc, owner)
	{
	}

	virtual ~graphics() { }

	virtual void set_mode(int width, int height, int bpp) = 0;
	virtual void reset() = 0;

	virtual void *fb() const = 0;
	virtual void blit(const void *src, size_t size) = 0;
};
} // namespace stacsos::kernel::dev::gfx
