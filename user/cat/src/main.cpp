/* SPDX-License-Identifier: MIT */

/* StACSOS - cat utility
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>

using namespace stacsos;

int main(const char *cmdline)
{
	if (!cmdline || memops::strlen(cmdline) == 0) {
		console::get().write("error: usage: cat <filename>\n");
		return 1;
	}

	object *file = object::open(cmdline);
	if (!file) {
		console::get().writef("error: unable to open file '%s' for reading\n", cmdline);
		return 1;
	}

	char buffer[64];
	int bytes_read;

	do {
		bytes_read = file->read(buffer, sizeof(buffer) - 1);
		buffer[bytes_read] = 0;

		console::get().writef("%s", buffer);
	} while (bytes_read > 0);

	delete file;
	return 0;
}
