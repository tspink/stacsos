/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
extern "C" void __stack_chk_fail()
{
	for (;;) { }
}

void panic(const char *msg, ...)
{
	for (;;)
		;
	__unreachable();
}
