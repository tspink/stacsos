/* SPDX-License-Identifier: MIT */

/* StACSOS - shell
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/process.h>

using namespace stacsos;

static void run_command(const char *cmd)
{
	// printf("Running Command: %s\n", cmd);

	char prog[64];
	int n = 0;
	while (*cmd && *cmd != ' ' && n < 63) {
		prog[n++] = *cmd++;
	}

	prog[n] = 0;

	if (*cmd)
		cmd++;

	auto pcmd = process::create(prog, cmd);
	if (!pcmd) {
		console::get().writef("error: unable to run program '%s'\n", prog);
	} else {
		pcmd->wait_for_exit();
	}
}

int main(void)
{
	console::get().write("This is the StACSOS shell.  Path resolution is \e\x0fnot-yet-implemented\e\x07, so you\n"
						 "must type the command EXACTLY.\n\n");

	console::get().write("Use the cat program to view the README: /usr/cat /docs/README\n\n");

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
