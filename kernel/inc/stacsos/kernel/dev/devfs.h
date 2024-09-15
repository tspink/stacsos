/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>
#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/fs-node.h>

namespace stacsos::kernel::dev {
using namespace stacsos::kernel::fs;

class devfs_node;
class device;

class devfs : public filesystem {
public:
	devfs();

	virtual ~devfs() { }
	virtual fs_node &root() override { return *(fs_node *)root_; }

private:
	devfs_node *root_;
};

class devfs_node : public fs_node {
	friend class devfs;

public:
	devfs_node(filesystem &fs, fs_node *parent, fs_node_kind kind, const string &name, device *dev)
		: fs_node(fs, parent, kind, name)
		, dev_(dev)
	{
	}

	virtual shared_ptr<file> open() override
	{
		if (dev_) {
			return dev_->open_as_file();
		} else {
			return nullptr;
		}
	}
	virtual fs_node *mkdir(const char *name) override { return nullptr; }

protected:
	virtual fs_node *resolve_child(const string &name) override;

private:
	device *dev_;
};
} // namespace stacsos::kernel::dev
