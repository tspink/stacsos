/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/devfs.h>
#include <stacsos/kernel/dev/device-manager.h>

using namespace stacsos;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::fs;

devfs::devfs()
	: root_(new devfs_node(*this, nullptr, fs_node_kind::directory, "", nullptr))
{
}

fs_node *devfs_node::resolve_child(const string &name)
{
	if (dev_ != nullptr) {
		return nullptr;
	}

	dprintf("devfs: resolve %s\n", name.c_str());

	device *dp;
	if (!device_manager::get().try_get_device_by_name(name, dp)) {
		return nullptr;
	}

	return new devfs_node(fs(), this, fs_node_kind::file, name, dp);
}
