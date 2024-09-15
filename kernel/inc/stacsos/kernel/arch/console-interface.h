/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch {
class console_interface {
public:
	console_interface()
		: cur_attr_(0x07)
	{
	}

	virtual void write_char(unsigned char c, u8 attr) = 0;

	void write(const char *text)
	{
		while (*text) {
			if (*text == '\e') {
				text++;

				switch (*text) {
				case 0:
					return;
				case '0' ... '9':
					cur_attr_ = *text - '0';
					break;
				case 'a' ... 'f':
					cur_attr_ = (*text - 'a') + 0xa;
					break;
				}

				text++;
			}

			write_char(*text++, cur_attr_);
		}
	}

	void write_line(const char *text)
	{
		write(text);
		write_char('\n', 0);
	}

private:
	u8 cur_attr_;
};
} // namespace stacsos::kernel::arch
