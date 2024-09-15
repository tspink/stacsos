/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
enum class syscall_result_code : u64 { ok = 0, not_found = 1, not_supported = 2 };

enum class syscall_numbers {
	exit = 0,
	open = 1,
	close = 2,
	read = 3,
	write = 4,
	pwrite = 5,
	set_fs = 6,
	set_gs = 7,
	alloc_mem = 8,
	start_process = 9,
	wait_for_process = 10,
	start_thread = 11,
	join_thread = 12,
	sleep = 13,
	poweroff = 14
};

struct syscall_result {
	syscall_result_code code;
	u64 data;
} __packed;

} // namespace stacsos
