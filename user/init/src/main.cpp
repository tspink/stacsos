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

// #define GFX_LOGO

static void logo()
{
	object *fb = object::open("/dev/virtcon0");

	int console_mode = (int)fb->ioctl(1, nullptr, 0);
	if (console_mode == 0) {
		console::get().write("\e\xb0                                                                                ");
		console::get().write("\e\xb0                             Welcome to StACSOS!                                ");
		console::get().write("\e\xb0                                                                                ");
	} else {
		console::get().write("\n\n\n\n");

		auto logo_file = object::open("/logo.ppm");
		if (!logo_file) {
			console::get().write("logo not found\n");
		}

		u8 *logo_data = new u8[170415];
		logo_file->read(logo_data, 170415);
		delete logo_file;

		logo_data++; // 0x50 P
		logo_data++; // 0x36 6
		logo_data++; // 0x0a

		int width = 0;
		while (*logo_data != 0x20) {
			width *= 10;
			width += *logo_data++ - '0';
		}

		logo_data++;

		int height = 0;
		while (*logo_data != 0xa) {
			height *= 10;
			height += *logo_data++ - '0';
		}

		logo_data++;

		int maxval = 0;
		while (*logo_data != 0xa) {
			maxval *= 10;
			maxval += *logo_data++ - '0';
		}

		logo_data++;

		object *fb = object::open("/dev/virtcon0");
		const u8 *pixel_data = (const u8 *)logo_data;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				u32 data_pixel_offset = (x + (y * width)) * 3;
				u32 screen_pixel_offset = (x + ((640 - width) / 2)) + (y * 640);

				u32 pixel = pixel_data[data_pixel_offset] << 16 | pixel_data[data_pixel_offset + 1] << 8 | pixel_data[data_pixel_offset + 2] << 0;

				fb->pwrite((const void *)&pixel, 4, screen_pixel_offset);
			}
		}
	}
}

int main(const char *cmdline)
{
	logo();

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
