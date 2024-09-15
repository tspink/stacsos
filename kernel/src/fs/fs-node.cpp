/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/fs-node.h>

using namespace stacsos::kernel::fs;

fs_node *fs_node::lookup(const char *path)
{
	// dprintf("fs: lookup: %s\n", path);

	// Paths from nodes MUST be relative.
	if (path[0] == '/') {
		return nullptr;
	}

	if (path[0] == '\0') {
		return mounted_fs_ ? &mounted_fs_->root() : this;
	}

	// If there is a mount on this node...
	if (mounted_fs_) {
		// dprintf("fs: traversing mount\n");
		return mounted_fs_->root().lookup(path);
	} else {
		// dprintf("fs: resolving child\n");
		char child_name[512];

		int index = 0;
		while (*path && *path != '/') {
			child_name[index++] = *path++;
		}
		child_name[index] = 0;

		auto *child = resolve_child(child_name);
		if (child) {
			if (*path == '\0') {
				return child;
			}

			// TODO: see if we need to skip the slash
			return child->lookup(++path);
		} else {
			return nullptr;
		}
	}
}
