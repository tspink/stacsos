/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/fs-node.h>
#include <stacsos/list.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::fs {
class fat_filesystem;
class fat_file;

class fat_file : public file {
public:
	fat_file(fat_filesystem &fs, u64 first_cluster, u64 file_size)
		: file(file_size)
		, fs_(fs)
		, clusters_(nullptr)
		, nr_clusters_(0)
	{
		read_cluster_list(first_cluster, file_size);
	}

	virtual ~fat_file() { delete[] clusters_; }

	virtual size_t pread(void *buffer, size_t offset, size_t length);
	virtual size_t pwrite(const void *buffer, size_t offset, size_t length);

private:
	void read_cluster_list(u64 first_cluster, u64 file_size);

	fat_filesystem &fs_;
	u64 *clusters_;
	u64 nr_clusters_;
};

class fat_node : public fs_node {
	friend class fat_filesystem;

public:
	fat_node(filesystem &fs, fs_node *parent, fs_node_kind kind, const string &name, u64 sector, u64 cluster, u64 data_size)
		: fs_node(fs, parent, kind, name)
		, sector_(sector)
		, cluster_(cluster)
		, data_size_(data_size)
		, loaded_(false)
	{
	}

	virtual ~fat_node() { }

	virtual shared_ptr<file> open() override { return shared_ptr<file>(new fat_file((fat_filesystem &)fs(), cluster_, data_size_)); }
	virtual fs_node *mkdir(const char *name) override;

protected:
	virtual fs_node *resolve_child(const string &name) override;

private:
	void load();

	u64 sector_, cluster_;
	u64 data_size_;
	bool loaded_;
	list<fat_node *> children_;
};

class fat_filesystem : public physical_filesystem {
	friend class fat_node;
	friend class fat_file;

public:
	fat_filesystem(dev::storage::block_device &bdev)
		: physical_filesystem(bdev)
		, root_(*this, nullptr, fs_node_kind::directory, "", 0, 0, 0)
	{
		init();
	}

	virtual ~fat_filesystem() { }

	virtual fs_node &root() override { return root_; }

private:
	void init();

	u64 compute_sector_for_cluster(u64 cluster) { return ((cluster - 2) * sectors_per_cluster) + first_data_sector; }

	shared_ptr<u8> read_cluster(u64 cluster) { return read_cluster_from_sector(compute_sector_for_cluster(cluster)); }
	shared_ptr<u8> read_cluster_from_sector(u64 sector);

	u64 next_cluster(u64 this_cluster);

	fat_node root_;

	u64 total_sectors;
	u64 fat_size;
	u64 root_dir_sectors;
	u64 first_fat_sector;
	u64 first_data_sector;
	u64 data_sectors;
	u64 total_clusters;
	u64 sectors_per_cluster;
};
} // namespace stacsos::kernel::fs
