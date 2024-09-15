/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/fs-node.h>
#include <stacsos/list.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::fs {
class tar_filesystem;

struct tar_file_header {
	char file_path[100];
	char file_mode[8];
	char owner_user_id[8];
	char owner_group_id[8];
	char file_size[12];
	char file_mtime[12];
	char header_checksum[8];
	char file_type;
	char link_path[100];

	char padding[255];
} __packed;

class tar_filesystem;
class tarfs_file : public file {
public:
	tarfs_file(tar_filesystem &fs, u64 data_start, u64 file_size)
		: file(file_size)
		, fs_(fs)
		, data_start_(data_start)
	{
	}

	virtual size_t pread(void *buffer, size_t offset, size_t length);
	virtual size_t pwrite(const void *buffer, size_t offset, size_t length);

private:
	tar_filesystem &fs_;
	u64 data_start_;

	void read_file_blocks(void *buffer, u64 offset, u64 count);
};

class tarfs_node : public fs_node {
	friend class tar_filesystem;

public:
	tarfs_node(filesystem &fs, fs_node *parent, fs_node_kind kind, const string &name, u64 data_start, u64 data_size)
		: fs_node(fs, parent, kind, name)
		, data_start_(data_start)
		, data_size_(data_size)
	{
	}

	virtual shared_ptr<file> open() override { return shared_ptr<file>(new tarfs_file((tar_filesystem &)fs(), data_start_, data_size_)); }
	virtual fs_node *mkdir(const char *name) override;

protected:
	virtual fs_node *resolve_child(const string &name) override;

private:
	tarfs_node *add_child(const string &name, fs_node_kind kind, u64 data_start, u64 data_size)
	{
		auto *node = new tarfs_node(fs(), this, kind, name, data_start, data_size);
		children_.append(node);
		return node;
	}

	bool has_child(const string &name);

	list<tarfs_node *> children_;
	u64 data_start_, data_size_;
};

class tar_filesystem : public physical_filesystem {
	friend class tarfs_file;

public:
	tar_filesystem(dev::storage::block_device &bdev)
		: physical_filesystem(bdev)
		, root_(*this, nullptr, fs_node_kind::directory, "", 0, 0)
	{
		load_tree();
	}

	virtual ~tar_filesystem() { }

	virtual fs_node &root() override { return root_; }

private:
	void load_tree();
	void register_file(const tar_file_header *header, u64 data_block_start, u64 data_size);

	tarfs_node root_;
};
} // namespace stacsos::kernel::fs
