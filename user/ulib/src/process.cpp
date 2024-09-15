/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/process.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

process *process::create(const char *path, const char *args)
{
	auto rc = syscalls::start_process(path, args);

	if (rc.code != syscall_result_code::ok) {
		return nullptr;
	}

	return new process(rc.data);
}

void process::wait_for_exit() { syscalls::wait_process(handle_); }
