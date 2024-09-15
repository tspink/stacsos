/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/block-device.h>
#include <stacsos/kernel/fs/tar-filesystem.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel::fs;

fs_node *tarfs_node::resolve_child(const string &name)
{
	// dprintf("tarfs: resolve child %s\n", name.c_str());

	for (auto *child : children_) {
		if (child->name() == name) {
			return child;
		}
	}

	return nullptr;
}

fs_node *tarfs_node::mkdir(const char *name)
{
	// dprintf("tarfs: mkdir %s\n", name);

	return add_child(string(name), fs_node_kind::directory, 0, 0);
}

size_t parse_octal(const char *str, size_t maxlen)
{
	size_t num = 0;
	for (size_t i = 0; i < maxlen && str[i] >= '0' && str[i] <= '7'; ++i) {
		num *= 8;
		num += str[i] - '0';
	}

	return num;
}

void tar_filesystem::load_tree()
{
	char buffer[512];

	u64 current_block = 0;
	u64 last_block = bdev_.nr_blocks();
	while (current_block < last_block) {
		bdev_.read_blocks_sync(buffer, current_block, 1);

		const tar_file_header *header = (const tar_file_header *)buffer;
		if (header->file_path[0] == 0) {
			break;
		}

		// Skip the header block
		current_block++;

		u64 size = parse_octal(header->file_size, 12);
		register_file(header, current_block, size);

		// Skip the file data blocks
		current_block += (((size + 511) >> 9));
	}
}

void tar_filesystem::register_file(const tar_file_header *header, u64 data_block_start, u64 data_size)
{
	const char *root_relative_name = &header->file_path[2];
	if (*root_relative_name == 0) {
		return;
	}

	list<string> path_components = string(root_relative_name).split('/', false);

	// dprintf("processing: %s\n", root_relative_name);

	tarfs_node *current_node = &root_;
	while (!path_components.empty()) {
		auto component = path_components.dequeue();

		// dprintf("  component: %s\n", component.c_str());

		if (path_components.empty()) {
			// This is the last component, i.e. the final file.
			tarfs_node *existing_child = (tarfs_node *)current_node->resolve_child(component);
			if (existing_child != nullptr) {
				panic("file already exists");
			}

			current_node->add_child(component, fs_node_kind::file, data_block_start, data_size);
			break;
		} else {
			// This is part of the path.
			tarfs_node *existing_child = (tarfs_node *)current_node->resolve_child(component);
			if (existing_child == nullptr) {
				existing_child = current_node->add_child(component, fs_node_kind::directory, 0, 0);
			}

			current_node = existing_child;
		}
	}
}

size_t tarfs_file::pread(void *buffer, size_t offset, size_t length)
{
	// dprintf("tarfs: pread: offset=%d len=%d\n", offset, length);

	size_t orig_length = length;
	u64 current_file_block = offset / 512;
	u64 start_block_offset = offset % 512;

	u8 *output_ptr = (u8 *)buffer;
	while (length) {
		// dprintf("tarfs: pread: block=%d len=%d\n", current_file_block, length);
		//   Need to read into a buffer, in case length < buffer size
		char block_buffer[512];
		read_file_blocks(block_buffer, current_file_block, 1);

		u64 amount_to_copy = min(length, (size_t)(sizeof(block_buffer) - start_block_offset));
		// dprintf("tarfs: atc: %lu, sbo: %lu\n", amount_to_copy, start_block_offset);

		memops::memcpy(output_ptr, block_buffer + start_block_offset, amount_to_copy);

		length -= amount_to_copy;
		output_ptr += amount_to_copy;
		current_file_block++;
		start_block_offset = 0;
	}

	return orig_length;
}

size_t tarfs_file::pwrite(const void *buffer, size_t offset, size_t length) { return 0; }

void tarfs_file::read_file_blocks(void *buffer, u64 offset, u64 count) { fs_.bdev_.read_blocks_sync(buffer, data_start_ + offset, count); }
