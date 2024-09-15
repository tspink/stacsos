/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/memory.h>
#include <stacsos/string.h>

namespace stacsos::kernel::fs {

enum class fs_node_kind { file, directory };

class filesystem;
class file;

class fs_node {
public:
	fs_node(filesystem &parent_fs, fs_node *parent_node, fs_node_kind kind, const string &name)
		: fs_(parent_fs)
		, parent_node_(parent_node)
		, kind_(kind)
		, mounted_fs_(nullptr)
		, name_(name)
	{
	}

	void mount(filesystem &fs) { mounted_fs_ = &fs; }
	void umount() { mounted_fs_ = nullptr; }

	fs_node_kind kind() const { return kind_; }

	fs_node *lookup(const char *path);

	filesystem &fs() const { return fs_; }

	const string &name() const { return name_; }

	virtual shared_ptr<file> open() = 0;
	virtual fs_node *mkdir(const char *name) = 0;

protected:
	virtual fs_node *resolve_child(const string &name) { return nullptr; }

private:
	filesystem &fs_;
	fs_node *parent_node_;
	fs_node_kind kind_;
	filesystem *mounted_fs_;
	string name_;
};
} // namespace stacsos::kernel::fs
