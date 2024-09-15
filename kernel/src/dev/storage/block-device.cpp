/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/dev/storage/block-device.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;

device_class block_device::block_device_class(device_class::root, "blk");
