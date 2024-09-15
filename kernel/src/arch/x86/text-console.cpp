/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/arch/x86/text-console.h>

using namespace stacsos::kernel::arch::x86;

/**
 * @brief Initialises the text console.
 */
void text_console::init()
{
	// Reset the "current" X and Y coordinates.
	x_ = 0;
	y_ = 0;

	// Enable the cursor display, and update its position.
	enable_cursor();
	update_cursor();

	// Loop over the text console framebuffer, and clear it.
	for (int i = 0; i < cells; i++) {
		vram_[i] = 0;
	}
}

/**
 * @brief Writes the given character to the text console at the current cursor position.
 *
 * @param c The character to write to the console.
 * @param attr The formatting attribute of the character.
 */
void text_console::write_char(unsigned char c, u8 attr)
{
	// Write the character out to the QEMU debug stream, so that we get information
	// in the terminal, without having to use the graphical interface.
	ioports::qemu_debug_out::write8(c);

	// If the character is *not* a newline character, write it directly into
	// the framebuffer, applying the attribute.
	if (c != '\n') {
		vram_[x_ + (y_ * cols)] = ((u16)attr << 8) | c;
		x_++; // Also increment the X coordinate, i.e. moving the cursor right.
	}

	// If the X coordinate has exceeded the width of the framebuffer, OR
	// the character being written is a newline character...
	if (x_ >= cols || c == '\n') {
		// Reset the X coordinate to the left-hand-side of the framebuffer,
		// and increment the Y coordinate.
		x_ = 0;
		y_++;

		// If the Y coordinate has exceeded the height of the framebuffer...
		if (y_ >= rows) {
			scroll_last_line(); // Scroll the framebuffer by one line
			y_ = rows - 1; // Reset the Y coordinate
		}
	}

	// After all that, update the cursor position.
	update_cursor();
}

/**
 * @brief Enables the display of the flashing cursor on the framebuffer.
 */
void text_console::enable_cursor()
{
	ioports::console_control::write8(0x0a);
	ioports::console_data::write8((ioports::console_data::read8() & 0xc0) | 0xd);

	ioports::console_control::write8(0x0b);
	ioports::console_data::write8((ioports::console_data::read8() & 0xe0) | 0xf);
}

/**
 * @brief Updates the position of the flashing cursor, so that it matches what
 * our internal representation of the "current" coordinates are.
 */
void text_console::update_cursor()
{
	u16 pos = (y_ * cols) + x_;

	ioports::console_control::write8(0x0f);
	ioports::console_data::write8((u8)(pos & 0xff)); // Low 8-bits
	ioports::console_control::write8(0x0e);
	ioports::console_data::write8((u8)((pos >> 8) & 0xff)); // High 8-bits
}

/**
 * @brief Scrolls the framebuffer up by one line.
 */
void text_console::scroll_last_line()
{
	// Copy everything from Line 2 (1-indexed) onwards back up by one line.
	for (int i = cols; i < cols * rows; i++) {
		vram_[i - cols] = vram_[i];
	}

	// Clear the last line.
	for (int i = cols * (rows - 1); i < cols * rows; i++) {
		vram_[i] = 0x0700;
	}
}
