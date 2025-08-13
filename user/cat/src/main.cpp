/* SPDX-License-Identifier: MIT */

/* StACSOS - cat utility, with extremely basic formatting utility.
 *
 * Copyright (c) University of St Andrews 2024, 2025
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>

using namespace stacsos;

int main(const char *cmdline)
{
	if (!cmdline || memops::strlen(cmdline) == 0) {
		console::get().write("error: usage: cat [-f] <filename>\n");
		return 1;
	}

	bool formatting_mode = false;
	while (*cmdline) {
		if (*cmdline == '-') {
			cmdline++;

			if (*cmdline++ == 'f') {
				formatting_mode = true;
			} else {
				console::get().write("error: usage: cat [-f] <filename>\n");
				return 1;
			}
		} else {
			break;
		}
	}

	while (*cmdline == ' ') {
		cmdline++;
	};

	object *file = object::open(cmdline);
	if (!file) {
		console::get().writef("error: unable to open file '%s' for reading\n", cmdline);
		return 1;
	}

	char buffer[64];
	int bytes_read;

	bool coloured = false;

	do {
		bytes_read = file->read(buffer, sizeof(buffer) - 1);
		buffer[bytes_read] = 0;

		if (formatting_mode) {
			char *ch = &buffer[0];
			while (*ch) {
				if (*ch == '`') {
					coloured = !coloured;
					if (coloured) {
						console::get().write("\e\x0e");
					}

					console::get().writef("%c", *ch++);

					if (!coloured) {
						console::get().write("\e\x07");
					}
				} else {
					console::get().writef("%c", *ch++);
				}
			}
		} else {
			console::get().writef("%s", buffer);
		}
	} while (bytes_read > 0);

	delete file;
	return 0;
}
