/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/console-interface.h>

namespace stacsos::kernel::arch::x86 {
class text_console : public console_interface {
public:
	static const int rows = 25;
	static const int cols = 80;
	static const int cells = rows * cols;
	static const int size = cells * 2;

	text_console()
		: vram_((u16 *)(0xffff'8000'000b'8000ull))
		, x_(0)
		, y_(0)
	{
	}

	void init();

	virtual void write_char(unsigned char c, u8 attr) override;

private:
	u16 *vram_;
	int x_, y_;

	void enable_cursor();
	void update_cursor();
	void scroll_last_line();
};
} // namespace stacsos::kernel::arch::x86
