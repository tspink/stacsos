/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
class object {
public:
	static object *open(const char *path);

	virtual ~object();

	size_t write(const void *buffer, size_t length);
	size_t pwrite(const void *buffer, size_t length, size_t offset);

	size_t read(void *buffer, size_t length);
	size_t pread(void *buffer, size_t length, size_t offset);

	u64 ioctl(u64 cmd, void *buffer, size_t length);

private:
	u64 handle_;

	object(u64 handle)
		: handle_(handle)
	{
	}
};
} // namespace stacsos
