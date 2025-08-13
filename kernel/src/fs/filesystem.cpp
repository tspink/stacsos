/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/dev/storage/block-device.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/tar-filesystem.h>
#include <stacsos/kernel/fs/fat.h>

using namespace stacsos::kernel::fs;
using namespace stacsos::kernel::dev::storage;

filesystem *filesystem::create_from_bdev(block_device &bdev, fs_type_hint hint)
{
	if (hint == fs_type_hint::tarfs) {
		return new tar_filesystem(bdev);
	} else if (hint == fs_type_hint::fat) {
		return new fat_filesystem(bdev);
	}

	return nullptr;
}
