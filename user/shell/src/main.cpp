/* SPDX-License-Identifier: MIT */

/* StACSOS - shell
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/process.h>
#include <stacsos/string.h>

using namespace stacsos;

static void run_command(const char *cmd)
{
	string binary_name;
	while (*cmd && *cmd != ' ') {
		binary_name += *cmd++;
	}

	if (*cmd) {
		cmd++;
	}

	// If prog doesn't start with a '/', then search in /usr first...
	if (binary_name[0] != '/') {
		binary_name = "/usr/" + binary_name;
	}

	auto pcmd = process::create(binary_name.c_str(), cmd);

	if (!pcmd) {
		console::get().writef("error: unable to run program '%s'\n", binary_name.c_str());
	} else {
		pcmd->wait_for_exit();
	}
}

int main(void)
{
	console::get().write("This is the StACSOS shell.  Only basic path resolution is implemented: commands\n"
						 "without a leading \e\x0b/\e\x07 will attempt to resolve to \e\x0b/usr\e\x07.  Otherwise, you must type\nthe command EXACTLY.\n\n");

	console::get().write("Use the\e\x0b cat\e\x07 program to view the README:\e\x0b cat /docs/README\e\x07\n");
	console::get().write("                              Also try:\e\x0b cat -f /docs/README\e\x07\n");
	console::get().write("Use the\e\x0b poweroff\e\x07 program to exit: \e\x0bpoweroff\e\x07\n\n");

	while (true) {
		console::get().write("> ");

		char command_buffer[128];
		int n = 0;

		while (n < 127) {
			char c = console::get().read_char();
			if (c == 0)
				continue;
			if (c == '\n')
				break;
			if (c == '\b') {
				if (n > 0) {
					command_buffer[--n] = 0;
					console::get().write("\b");
				}
			} else {
				command_buffer[n++] = c;
				console::get().writef("%c", c);
			}
		}

		console::get().write("\n");
		if (n == 0)
			continue;

		command_buffer[n] = 0;

		if (memops::strcmp("exit", command_buffer) == 0)
			break;

		run_command(command_buffer);
	}

	return 0;
}
