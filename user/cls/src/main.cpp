/* SPDX-License-Identifier: MIT */

/* StACSOS - cat utility, with extremely basic formatting utility.
 *
 * Copyright (c) University of St Andrews 2024, 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/objects.h>

using namespace stacsos;

int main(const char *cmdline)
{
	console::get().clear();
	return 0;
}
