/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>
#include <stacsos/kernel/dev/input/keys.h>
#include <stacsos/kernel/sched/event.h>
#include <stacsos/memops.h>

namespace stacsos::kernel::dev::console {
enum class virtual_console_mode { text, gfx };

enum class keyboard_modifiers { none = 0, shift = 1 };

DEFINE_ENUM_FLAG_OPERATIONS(keyboard_modifiers)

class virtual_console_file;
class virtual_console : public device {
	friend class physical_console;
	friend class virtual_console_file;

public:
	constexpr static int TEXT_MODE_ROWS = 25;
	constexpr static int TEXT_MODE_COLS = 80;
	constexpr static int TEXT_MODE_CELLS = TEXT_MODE_ROWS * TEXT_MODE_COLS;

	constexpr static int GFX_MODE_WIDTH = 640;
	constexpr static int GFX_MODE_HEIGHT = 480;
	constexpr static int GFX_MODE_PIXELS = GFX_MODE_WIDTH * GFX_MODE_HEIGHT;

	static device_class virtual_console_device_class;

	virtual_console(bus &owner, virtual_console_mode mode)
		: device(virtual_console_device_class, owner)
		, mode_(mode)
		, internal_buffer_(nullptr)
		, internal_buffer_size_(0)
		, x_(0)
		, y_(0)
		, rows_(0)
		, cols_(0)
		, current_mod_mask_(keyboard_modifiers::none)
		, active_(false)
		, read_buffer_head_(0)
		, read_buffer_tail_(0)
	{
		switch (mode) {
		case virtual_console_mode::text:
			internal_buffer_size_ = TEXT_MODE_CELLS * 2;
			break;

		case virtual_console_mode::gfx:
			internal_buffer_size_ = GFX_MODE_PIXELS * 4;
			break;
		}

		internal_buffer_ = new u8[internal_buffer_size_];

		clear();
	}

	virtual ~virtual_console() { delete internal_buffer_; }

	virtual void configure() override;

	virtual_console_mode mode() const { return mode_; }

	void on_key_up(input::keys key);
	void on_key_down(input::keys key);

	void activate();
	void deactivate() { active_ = false; }

	void write_char(unsigned char ch, u8 attr);
	u8 read_char();

	void clear();

	virtual shared_ptr<fs::file> open_as_file() override;

private:
	virtual_console_mode mode_;
	u8 *internal_buffer_;
	size_t internal_buffer_size_;
	int x_, y_, rows_, cols_;
	keyboard_modifiers current_mod_mask_;
	bool active_;

	u8 read_buffer_[256];
	u8 read_buffer_head_, read_buffer_tail_;
	sched::event read_buffer_event_;

	void render_char(int x, int y, unsigned char ch, u8 attr);
	void update_cursor();
};
} // namespace stacsos::kernel::dev::console
