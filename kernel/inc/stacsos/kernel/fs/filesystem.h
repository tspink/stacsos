/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/fs/fs-node.h>
#include <stacsos/kernel/fs/file.h>

namespace stacsos::kernel::dev::storage {
class block_device;
}

namespace stacsos::kernel::fs {

enum class fs_type_hint { best_guess, tarfs, fat };

class filesystem {
public:
	virtual fs_node &root() = 0;

	virtual ~filesystem() { }
	static filesystem *create_from_bdev(dev::storage::block_device &bdev, fs_type_hint hint);
};

class physical_filesystem : public filesystem {
public:
	physical_filesystem(dev::storage::block_device &bdev)
		: bdev_(bdev)
	{
	}

	virtual ~physical_filesystem() { }

protected:
	dev::storage::block_device &bdev_;
};

class rootfs_node : public fs_node {
public:
	rootfs_node(filesystem &parent_fs)
		: fs_node(parent_fs, nullptr, fs_node_kind::directory, "")
	{
	}

	virtual shared_ptr<file> open() override { return nullptr; }
	virtual fs_node *mkdir(const char *name) override { return nullptr; }
};

class root_filesystem : public filesystem {
public:
	root_filesystem()
		: root_(*this)
	{
	}

	virtual ~root_filesystem() { }

	virtual fs_node &root() override { return root_; }

private:
	rootfs_node root_;
};
} // namespace stacsos::kernel::fs
