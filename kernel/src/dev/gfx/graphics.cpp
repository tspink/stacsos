/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/dev/gfx/graphics.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::gfx;

device_class graphics::graphics_device_class(device_class::root, "gfx");
