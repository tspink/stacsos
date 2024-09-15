/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/fs/filesystem.h>

namespace stacsos::kernel::fs {

class vfs {
	DEFINE_SINGLETON(vfs)

private:
	vfs() { }

	root_filesystem rootfs_;

public:
	void init();
	fs_node *lookup(const char *path);
};
} // namespace stacsos::kernel::fs
