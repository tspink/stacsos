/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/fs/vfs.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::fs;

void vfs::init()
{
	//
}

fs_node *vfs::lookup(const char *path)
{
	// dprintf("vfs: lookup '%s'\n", path);

	// All paths MUST be absolute.  To avoid any confusion.
	if (path[0] != '/') {
		return nullptr;
	}

	return rootfs_.root().lookup(&path[1]);
}
