/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
class process {
public:
	static process *create(const char *path, const char *args);

	void wait_for_exit();

private:
	process(u64 handle)
		: handle_(handle)
	{
	}

	u64 handle_;
};
} // namespace stacsos
