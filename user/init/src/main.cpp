/* SPDX-License-Identifier: MIT */

/* StACSOS - system initialisation
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/objects.h>
#include <stacsos/process.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

int main(const char *cmdline)
{
	console::get().write("\e\xb0                                                                                ");
	console::get().write("\e\xb0                             Welcome to StACSOS!                                ");
	console::get().write("\e\xb0                                                                                ");
	console::get().write("\e\x07\nSwitch virtual consoles by pressing Alt+F{1,2}\n\n");
	console::get().write("\e\x0cStarting the shell...\e\x07\n\n");

	while (true) {
		auto shell_proc = process::create("/usr/shell", "");
		if (shell_proc == nullptr) {
			console::get().write("ERROR: Unable to launch shell.\n");
			return 1;
		}

		shell_proc->wait_for_exit();
		delete shell_proc;

		console::get().write("Shell has \e\x04terminated\e\x07.  PRESS ENTER TO RESTART...\n");
		while (console::get().read_char() != '\n')
			;
	}

	return 0;
}
