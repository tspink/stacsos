/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::fs {
class filesystem;
class file {
public:
	file(u64 size)
		: size_(size)
		, cur_offset_(0)
	{
	}

	virtual ~file() { }

	virtual u64 ioctl(u64 cmd, void *buffer, size_t length) { return 0; }

	virtual size_t pread(void *buffer, size_t offset, size_t length) = 0;
	virtual size_t pwrite(const void *buffer, size_t offset, size_t length) = 0;

	virtual size_t read(void *buffer, size_t length)
	{
		u64 read_length = length;
		if ((cur_offset_ + read_length) > size_) {
			read_length = size_ - cur_offset_;
		}

		size_t result = pread(buffer, cur_offset_, read_length);
		cur_offset_ += result;

		return result;
	}

	virtual size_t write(const void *buffer, size_t length)
	{
		u64 write_length = length;
		if ((cur_offset_ + write_length) > size_) {
			write_length = size_ - cur_offset_;
		}

		size_t result = pwrite(buffer, cur_offset_, write_length);
		cur_offset_ += result;

		return result;
	}

private:
	u64 size_;
	u64 cur_offset_;
};
} // namespace stacsos::kernel::fs
