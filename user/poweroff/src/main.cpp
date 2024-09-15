/* SPDX-License-Identifier: MIT */

/* StACSOS - poweroff utility
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/user-syscall.h>

using namespace stacsos;

int main(const char *cmdline)
{
	syscalls::poweroff();
	return 0;
}
