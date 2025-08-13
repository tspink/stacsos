/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/dev/storage/partitioned-device.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;

device_class partition::partition_device_class(block_device::block_device_class, "part");
