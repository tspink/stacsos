/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/block-device.h>
#include <stacsos/kernel/fs/fat.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel::fs;

struct bios_parameter_block {
	u8 code[3];
	u8 oam_id[8];
	u16 bytes_per_sector;
	u8 sectors_per_cluster;
	u16 nr_reserved_sectors;
	u8 nr_fats;
	u16 nr_root_dentries;
	u16 total_sectors;
	u8 media_descriptor;
	u16 sectors_per_fat;
	u16 sectors_per_track;
	u16 nr_heads;
	u32 nr_hidden_sectors;
	u32 nr_large_sectors;
} __packed;

struct fat12_ebr {
	u8 drive_number;
	u8 nt_flags;
	u8 signature;
	u32 volume_id;
	u8 volume_label[11];
	u8 system_id[8];
	u8 boot_code[448];
	u16 partition_signature;
} __packed;

struct fat32_ebr {
	u32 sectors_per_fat;
	u16 flags;
	u16 fat_version;
	u32 cluster_of_root_dir;
	u16 fs_info_sector;
	u16 backup_boot_sector;
	u8 reserved[12];
	u8 drive_number;
	u8 nt_flags;
	u8 signature;
	u32 volume_id;
	u8 volume_label[11];
	u8 system_id[8];
	u8 boot_code[420];
	u16 partition_signature;
} __packed;

void fat_filesystem::init()
{
	auto mbuffer = shared_ptr((u8 *)new u8[512]);
	auto buffer = mbuffer.get();

	dprintf("fat: init\n");

	bdev_.read_blocks_sync(buffer, 0, 1);

	dprintf("fat: magic: %02x %02x\n", buffer[510], buffer[511]);
	if (buffer[510] != 0x55 || buffer[511] != 0xaa) {
		panic("fat: not a FAT volume");
		return;
	}

	const bios_parameter_block *bpb = (const bios_parameter_block *)&buffer[0];

	// FAT metric computation
	total_sectors = bpb->total_sectors == 0 ? bpb->nr_large_sectors : bpb->total_sectors;
	fat_size = bpb->sectors_per_fat;
	root_dir_sectors = ((bpb->nr_root_dentries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
	first_fat_sector = bpb->nr_reserved_sectors;
	first_data_sector = first_fat_sector + (bpb->nr_fats * fat_size) + root_dir_sectors;
	data_sectors = total_sectors - first_data_sector;
	sectors_per_cluster = bpb->sectors_per_cluster;
	total_clusters = data_sectors / sectors_per_cluster;

	dprintf("fat: total-sectors=%lu\n", total_sectors);
	dprintf("fat: fat-size=%lu\n", fat_size);
	dprintf("fat: root-dir-sectors=%lu\n", root_dir_sectors);
	dprintf("fat: first-fat-sector=%lu\n", first_fat_sector);
	dprintf("fat: first-data-sector=%lu\n", first_data_sector);
	dprintf("fat: data-sectors=%lu\n", data_sectors);
	dprintf("fat: sectors-per-cluster=%lu\n", sectors_per_cluster);
	dprintf("fat: total-clusters=%lu\n", total_clusters);

	// FAT type identification
	if (total_clusters < 4085) {
		panic("FAT12 not supported");
	} else if (total_clusters < 65525) {
		dprintf("fat: fat16\n");
	} else {
		panic("FAT32 not supported");
	}

	const fat12_ebr *ebr = (const fat12_ebr *)&buffer[0x24];

	if (ebr->signature != 0x29) {
		panic("fat: invalid FAT signature");
	}

	dprintf("fat: signature=%2x\n", ebr->signature);

	char volume_label[sizeof(ebr->volume_label) + 1] = { 0 };
	stacsos::memops::memcpy(volume_label, ebr->volume_label, sizeof(ebr->volume_label));
	dprintf("fat: volume-label=%s\n", volume_label);

	root_.sector_ = first_data_sector - root_dir_sectors;
}

shared_ptr<u8> fat_filesystem::read_cluster_from_sector(u64 sector)
{
	shared_ptr<u8> buffer = shared_ptr<u8>(new u8[512 * sectors_per_cluster]);

	bdev_.read_blocks_sync(buffer.get(), sector, sectors_per_cluster);
	return buffer;
}

u64 fat_filesystem::next_cluster(u64 this_cluster)
{
	u32 fat_offset = this_cluster * 2;
	u32 fat_sector = first_fat_sector + (fat_offset / 512);
	u32 entry_offset = fat_offset % 512;

	auto fat_table = read_cluster_from_sector(fat_sector);
	u16 value = *(u16 *)&fat_table.get()[entry_offset];

	return value;
}

fs_node *fat_node::mkdir(const char *name)
{
	auto new_dir = new fat_node(fs(), this, fs_node_kind::directory, string(name), 0, 0, 0);
	children_.append(new_dir);

	return new_dir;
}

fs_node *fat_node::resolve_child(const string &name)
{
	load();

	for (auto child : children_) {
		if (child->name() == name) {
			return child;
		}
	}

	return nullptr;
}

void fat_node::load()
{
	if (loaded_) {
		return;
	}

	fat_filesystem &fatfs = ((fat_filesystem &)fs());

	u64 this_cluster = cluster_;

	do {
		// Read in the current cluster data.
		auto cluster_data = fatfs.read_cluster(this_cluster);

		// Parse dentries from this cluster
		char long_filename_buffer[256];
		long_filename_buffer[0] = 0;

		bool has_long_filename = false;

		for (const u8 *dentry = &(cluster_data.get())[0]; dentry < &(cluster_data.get())[512 * fatfs.sectors_per_cluster]; dentry += 32) {
			if (dentry[0] == 0) {
				// No more files in this directory.
				break;
			} else if (dentry[0] == 0xe5) {
				// Entry is unused -- ignore.
				continue;
			}

			if (dentry[11] == 0x0f) {
				has_long_filename = true;

				// Long filename record
				u8 filename_offset = 0;

				for (int i = 1; i < 11; i += 2) {
					long_filename_buffer[filename_offset++] = dentry[i];
				}

				for (int i = 14; i < 26; i += 2) {
					long_filename_buffer[filename_offset++] = dentry[i];
				}

				for (int i = 28; i < 32; i += 2) {
					long_filename_buffer[filename_offset++] = dentry[i];
				}

				continue;
			}

			char short_filename[12] = { 0 };
			memops::memcpy(short_filename, dentry, 11);
			for (int i = 0; i < 11; i++) {
				if (short_filename[i] == 0x20) {
					short_filename[i] = 0;
				} else if (short_filename[i] > 0x40 && short_filename[i] < 0x5b) {
					short_filename[i] |= 0x20;
				}
			}

			string filename;
			if (has_long_filename) {
				filename = string(long_filename_buffer);

				has_long_filename = false;
				memops::memset(long_filename_buffer, 0, sizeof(long_filename_buffer));
			} else {
				filename = string(short_filename);
			}

			u32 cluster = ((u32) * ((u16 *)&dentry[26])) | (((u32) * ((u16 *)&dentry[20])) << 16);
			u64 sector = ((cluster - 2) * fatfs.sectors_per_cluster) + fatfs.first_data_sector;
			u64 size = *(u32 *)&dentry[28];

			children_.append(new fat_node(fs(), this, (dentry[11] & 0x10) ? fs_node_kind::directory : fs_node_kind::file, filename, sector, cluster, size));
		}

		this_cluster = fatfs.next_cluster(this_cluster);
		if (this_cluster >= 0xfff8) {
			// No more clusters.
			break;
		}
	} while (true);

	loaded_ = true;
}

void fat_file::read_cluster_list(u64 first_cluster, u64 file_size)
{
	u64 cluster_size = (512 * fs_.sectors_per_cluster);
	nr_clusters_ = (file_size + (cluster_size - 1)) / cluster_size;

	clusters_ = new u64[nr_clusters_];

	u64 this_cluster = first_cluster;
	for (int i = 0; i < nr_clusters_; i++) {
		if (this_cluster >= 0xFFF8) {
			dprintf("fat: warning: not enough clusters for reported file size\n");
			nr_clusters_ = i - 1;
			break;
		}

		clusters_[i] = this_cluster;
		this_cluster = fs_.next_cluster(this_cluster);
	}
}

size_t fat_file::pread(void *buffer, size_t offset, size_t length)
{
	u64 cluster_size = (512 * fs_.sectors_per_cluster);
	u64 target_cluster_index = offset / cluster_size;
	u64 target_cluster_offset = offset % cluster_size;

	u64 this_cluster = clusters_[target_cluster_index++];

	u8 *buffer_pos = (u8 *)buffer;

	u64 remaining_length = length;
	while (remaining_length > 0) {
		u64 read_length = ((target_cluster_offset + remaining_length) > cluster_size) ? (cluster_size - target_cluster_offset) : remaining_length;

		auto cluster_data = fs_.read_cluster(this_cluster);

		memops::memcpy(buffer_pos, cluster_data.get() + target_cluster_offset, read_length);

		buffer_pos += read_length;
		remaining_length -= read_length;

		// Break early if we're done, so we don't unnecessarily traverse the FAT
		if (remaining_length == 0) {
			break;
		}

		// Locate the next cluter in this chain
		if (target_cluster_index >= nr_clusters_) {
			// No further clusters -- we're past the end of the file data.
			break;
		}

		this_cluster = clusters_[target_cluster_index++];
		target_cluster_offset = 0; // Start at the beginning of the next cluster.
	}

	return length - remaining_length;
}

size_t fat_file::pwrite(const void *buffer, size_t offset, size_t length) { return 0; }
