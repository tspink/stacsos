/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
using guard = u8;

extern "C" int __cxa_guard_acquire(guard *g)
{
	if (*g) {
		return 0;
	}

	*g = 1;
	return 1;
}

extern "C" void __cxa_guard_release(guard *g) { }

extern "C" void __cxa_guard_abort(guard *g) { }
