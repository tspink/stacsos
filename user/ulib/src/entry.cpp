/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/objects.h>
#include <stacsos/console.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

extern int main(const char *cmdline);

static char tls[256];

static void init_tls() { stacsos::syscalls::set_fs((u64)tls); }

extern "C" void start_main(const char *cmdline)
{
	init_tls();

	console::get().init();

	int rc = main(cmdline);

	stacsos::syscalls::exit((u64)rc);
	while (1) { }

	__unreachable();
}
