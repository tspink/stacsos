/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/console/console-font.h>
#include <stacsos/kernel/dev/console/virtual-console.h>
#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/sched/process-manager.h>
#include <stacsos/kernel/sched/sleeper.h>

using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::console;
using namespace stacsos::kernel::dev::input;
using namespace stacsos::kernel::fs;
using namespace stacsos::kernel::sched;
using namespace stacsos;

device_class virtual_console::virtual_console_device_class(device_class::root, "virtcon");

extern console_font *active_font;

void virtual_console::configure()
{
	switch (mode_) {
	case virtual_console_mode::text:
		rows_ = TEXT_MODE_ROWS;
		cols_ = TEXT_MODE_COLS;
		break;

	case virtual_console_mode::gfx:
		active_font->parse();
		rows_ = GFX_MODE_HEIGHT / active_font->char_dims().height();
		cols_ = GFX_MODE_WIDTH / active_font->char_dims().width();
		break;
	}
}

void virtual_console::on_key_up(keys key)
{
	// TODO: buggy if two shift/ctrl keys held down
	switch (key) {
	case keys::KEY_LSHIFT:
	case keys::KEY_RSHIFT:
		current_mod_mask_ = current_mod_mask_ & ~keyboard_modifiers::shift;
		return;

	case keys::KEY_LCTRL:
	case keys::KEY_RCTRL:
		current_mod_mask_ = current_mod_mask_ & ~keyboard_modifiers::ctrl;
		return;

	default:
		break;
	}
}

void virtual_console::on_key_down(keys key)
{
#define KEY_MAPPING(_kc, _upper, _lower)                                                                                                                       \
	case keys::_kc:                                                                                                                                            \
		if ((current_mod_mask_ & keyboard_modifiers::shift) == keyboard_modifiers::shift)                                                                      \
			ch = _upper;                                                                                                                                       \
		else                                                                                                                                                   \
			ch = _lower;                                                                                                                                       \
		break
	char ch = 0;
	switch (key) {
		KEY_MAPPING(KEY_A, 'A', 'a');
		KEY_MAPPING(KEY_B, 'B', 'b');
		KEY_MAPPING(KEY_C, 'C', 'c');
		KEY_MAPPING(KEY_D, 'D', 'd');
		KEY_MAPPING(KEY_E, 'E', 'e');
		KEY_MAPPING(KEY_F, 'F', 'f');
		KEY_MAPPING(KEY_G, 'G', 'g');
		KEY_MAPPING(KEY_H, 'H', 'h');
		KEY_MAPPING(KEY_I, 'I', 'i');
		KEY_MAPPING(KEY_J, 'J', 'j');
		KEY_MAPPING(KEY_K, 'K', 'k');
		KEY_MAPPING(KEY_L, 'L', 'l');
		KEY_MAPPING(KEY_M, 'M', 'm');
		KEY_MAPPING(KEY_N, 'N', 'n');
		KEY_MAPPING(KEY_O, 'O', 'o');
		KEY_MAPPING(KEY_P, 'P', 'p');
		KEY_MAPPING(KEY_Q, 'Q', 'q');
		KEY_MAPPING(KEY_R, 'R', 'r');
		KEY_MAPPING(KEY_S, 'S', 's');
		KEY_MAPPING(KEY_T, 'T', 't');
		KEY_MAPPING(KEY_U, 'U', 'u');
		KEY_MAPPING(KEY_V, 'V', 'v');
		KEY_MAPPING(KEY_W, 'W', 'w');
		KEY_MAPPING(KEY_X, 'X', 'x');
		KEY_MAPPING(KEY_Y, 'Y', 'y');
		KEY_MAPPING(KEY_Z, 'Z', 'z');

		KEY_MAPPING(KEY_0, ')', '0');
		KEY_MAPPING(KEY_1, '!', '1');
		KEY_MAPPING(KEY_2, '"', '2');
		KEY_MAPPING(KEY_3, '#', '3');
		KEY_MAPPING(KEY_4, '$', '4');
		KEY_MAPPING(KEY_5, '%', '5');
		KEY_MAPPING(KEY_6, '^', '6');
		KEY_MAPPING(KEY_7, '&', '7');
		KEY_MAPPING(KEY_8, '*', '8');
		KEY_MAPPING(KEY_9, '(', '9');

		KEY_MAPPING(KEY_HYPHEN, '_', '-');
		KEY_MAPPING(KEY_EQUALS, '+', '=');
		KEY_MAPPING(KEY_BACKSLASH, '|', '\\');
		KEY_MAPPING(KEY_FORSLASH, '?', '/');
		KEY_MAPPING(KEY_HASH, '~', '#');
		KEY_MAPPING(KEY_LSQBRACKET, '{', '[');
		KEY_MAPPING(KEY_RSQBRACKET, '}', ']');

		KEY_MAPPING(KEY_APOSTROPHE, '@', '\'');
		KEY_MAPPING(KEY_DOT, '>', '.');
		KEY_MAPPING(KEY_COMMA, '<', ',');
		KEY_MAPPING(KEY_SEMICOLON, ':', ';');

	case keys::KEY_SPACE:
		ch = ' ';
		break;

	case keys::KEY_RETURN:
		ch = '\n';
		break;

	case keys::KEY_BACKSPACE:
		ch = '\b';
		break;

	case keys::KEY_LSHIFT:
	case keys::KEY_RSHIFT:
		current_mod_mask_ = current_mod_mask_ | keyboard_modifiers::shift;
		return;

	case keys::KEY_LCTRL:
	case keys::KEY_RCTRL:
		current_mod_mask_ = current_mod_mask_ | keyboard_modifiers::ctrl;
		return;

	default:
		return;
	}

	read_buffer_[read_buffer_tail_++] = ch;
	read_buffer_tail_ %= ARRAY_SIZE(read_buffer_);
	read_buffer_event_.trigger();
}

static u32 vga_colour_map[] = {
	/* LOW */ 0x000000, 0x000080, 0x008000, 0x008080, 0x800000, 0x800080, 0x808000, 0x808080, //
	/* HI */ 0x404040, 0x0000ff, 0x00ff00, 0x00ffff, 0xff0000, 0xff00ff, 0xffff00, 0xffffff
};

void virtual_console::render_char(int x, int y, unsigned char ch, u8 attr)
{
	if (mode_ == virtual_console_mode::text) {
		u16 *text_buffer = (u16 *)internal_buffer_;
		text_buffer[x + (y * cols_)] = ((u16)attr << 8) | (u16)ch;
	} else if (mode_ == virtual_console_mode::gfx) {
		u32 fg_colour = vga_colour_map[attr & 0xf];
		u32 bg_colour = vga_colour_map[(attr >> 4) & 0xf];

		u32 *frame_buffer = (u32 *)internal_buffer_;

		console_font_char font_char = active_font->get_char(ch);
		unsigned int pixel_offset = (x * font_char.dims().width()) + (y * GFX_MODE_WIDTH * font_char.dims().height());
		// const char *char_data = &console_font_data[(int)ch * console_char_height];

		for (int cy = 0; cy < font_char.dims().height(); cy++) {
			for (int cx = 0; cx < font_char.dims().width(); cx++) {
				// frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = ((*char_data) & (1 << (7 - cx))) ? fg_colour : bg_colour;
				frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = font_char.get_pixel(cx, cy) ? fg_colour : bg_colour;
			}
		}
	}
}

void virtual_console::write_char(unsigned char ch, u8 attr)
{
	switch (ch) {
	case '\n':
		x_ = 0;
		y_++;
		break;

	case '\r':
		x_ = 0;
		break;

	case '\b':
		if (x_ > 0) {
			x_--;
			render_char(x_, y_, ' ', attr);
		}
		break;

	default:
		render_char(x_, y_, ch, attr);
		x_++;
		if (x_ >= cols_) {
			x_ = 0;
			y_++;
		}
		break;
	}

	if (y_ >= rows_) {
		if (mode_ == virtual_console_mode::text) {
			u16 *text_buffer = (u16 *)internal_buffer_;

			for (int i = cols_; i < rows_ * cols_; i++) {
				text_buffer[i - cols_] = text_buffer[i];
			}

			// Clear the last line.
			for (int i = cols_ * (rows_ - 1); i < rows_ * cols_; i++) {
				text_buffer[i] = 0x0720;
			}
		} else {
			u32 *frame_buffer = (u32 *)internal_buffer_;

			memops::memcpy(frame_buffer, &frame_buffer[GFX_MODE_WIDTH * active_font->char_dims().height()],
				((GFX_MODE_WIDTH * GFX_MODE_HEIGHT) - (GFX_MODE_WIDTH * active_font->char_dims().height())) * 4);
			memops::memset(&frame_buffer[((GFX_MODE_WIDTH * GFX_MODE_HEIGHT) - (GFX_MODE_WIDTH * active_font->char_dims().height()))], 0,
				(GFX_MODE_WIDTH * active_font->char_dims().height()) * 4);
		}
		y_ = rows_ - 1;
	}

	update_cursor();
}

void virtual_console::activate()
{
	active_ = true;

	if (mode_ == virtual_console_mode::gfx) {
		auto cft = process_manager::get().kernel_process()->create_thread((u64)cursor_flasher_thread_proc, this);
		cft->start();

		cursor_flasher_ = cft;
	}

	update_cursor();
}

void virtual_console::clear()
{
	x_ = y_ = 0;

	if (mode_ == virtual_console_mode::text) {
		u16 *text_buffer = (u16 *)internal_buffer_;

		for (int i = 0; i < 80 * 25; i++) {
			text_buffer[i] = 0x0720;
		}
	} else if (mode_ == virtual_console_mode::gfx) {
		u32 *frame_buffer = (u32 *)internal_buffer_;

		for (int i = 0; i < GFX_MODE_PIXELS; i++) {
			frame_buffer[i] = 0;
		}
	}

	update_cursor();
}

void virtual_console::cursor_flasher_thread_proc(void *arg)
{
	virtual_console *vc = (virtual_console *)arg;
	u32 *frame_buffer = (u32 *)vc->internal_buffer_;

	while (1) {
		unsigned int pixel_offset = (vc->x_ * 8) + (vc->y_ * GFX_MODE_WIDTH * 15);

		for (int cy = 14; cy < 15; cy++) {
			for (int cx = 0; cx < 8; cx++) {
				frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = 0x00ffffff;
			}
		}

		sleeper::get().sleep_ms(500);

		for (int cy = 14; cy < 15; cy++) {
			for (int cx = 0; cx < 8; cx++) {
				frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = 0;
			}
		}

		sleeper::get().sleep_ms(500);
	}
}

void virtual_console::update_cursor()
{
	if (!active_) {
		return;
	}

	if (mode_ == virtual_console_mode::text) {
		u16 pos = (y_ * cols_) + x_;

		ioports::console_control::write8(0x0f);
		ioports::console_data::write8((u8)(pos & 0xff)); // Low 8-bits
		ioports::console_control::write8(0x0e);
		ioports::console_data::write8((u8)((pos >> 8) & 0xff)); // High 8-bits
	} else {
		/*u32 *frame_buffer = (u32 *)internal_buffer_;

		unsigned int pixel_offset = (x_ * 8) + (y_ * GFX_MODE_WIDTH * 15);

		for (int cy = 14; cy < 15; cy++) {
			for (int cx = 0; cx < 8; cx++) {
				// frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = ((*char_data) & (1 << (7 - cx))) ? fg_colour : bg_colour;
				frame_buffer[pixel_offset + cx + (GFX_MODE_WIDTH * cy)] = 0x00ffffff;
			}
		}*/
	}
}

u8 virtual_console::read_char()
{
	while (read_buffer_head_ == read_buffer_tail_) {
		read_buffer_event_.wait();
	}

	u8 elem = read_buffer_[read_buffer_head_];

	read_buffer_head_++;
	read_buffer_head_ %= ARRAY_SIZE(read_buffer_);

	return elem;
}

namespace stacsos::kernel::dev::console {

class virtual_console_file : public file {
public:
	virtual_console_file(virtual_console &vc)
		: file(0)
		, vc_(vc)
	{
	}

	virtual size_t pread(void *buffer, size_t offset, size_t length) override
	{
		size_t clamped_length = length;

		if (vc_.mode() == virtual_console_mode::gfx) {
			memops::memcpy(buffer, &((u32 *)vc_.internal_buffer_)[offset], clamped_length);
		} else {
			memops::memcpy(buffer, &((u16 *)vc_.internal_buffer_)[offset], clamped_length);
		}

		return clamped_length;
	}

	virtual size_t pwrite(const void *buffer, size_t offset, size_t length) override
	{
		size_t clamped_length = length;

		if (vc_.mode() == virtual_console_mode::gfx) {
			memops::memcpy(&((u32 *)vc_.internal_buffer_)[offset], buffer, clamped_length);
		} else {
			memops::memcpy(&((u16 *)vc_.internal_buffer_)[offset], buffer, clamped_length);
		}

		return clamped_length;
	}

	virtual u64 ioctl(u64 cmd, void *buffer, size_t length) override
	{
		switch (cmd) {
		case 1:
			return (u64)vc_.mode();

		case 2:
			vc_.clear();
			return 0;

		default:
			return 0;
		}
	}

private:
	virtual_console &vc_;
};
} // namespace stacsos::kernel::dev::console

shared_ptr<file> virtual_console::open_as_file() { return shared_ptr(new virtual_console_file(*this)); }
