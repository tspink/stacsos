/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/string.h>

using namespace stacsos;

string string::pad(int width, char ch, pad_side side)
{
	if ((size_t)width <= size_)
		return *this;

	int nr_pad_chars = width - size_;

	char buffer[nr_pad_chars + 1];
	memops::memset(buffer, 0, sizeof(buffer));

	for (int i = 0; i < nr_pad_chars; i++) {
		buffer[i] = ch;
	}

	if (side == pad_side::LEFT) {
		return string(buffer) + *this;
	} else {
		return *this + string(buffer);
	}
}

string string::format(const string &fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	string output_str("");

	bool long_mode = false;
	bool has_pad = false;
	char pad_char = ' ';
	int pad_width = 0;

	const char *fmt_data = fmt.data_;
	while (*fmt_data) {
		switch (*fmt_data) {
		case '%':
			long_mode = false;
			has_pad = false;
			pad_char = ' ';
			pad_width = 0;

		retry_format:
			fmt_data++;
			switch (*fmt_data) {
			case '%':
				output_str += '%';
				break;

			case 'l':
				long_mode = true;
				goto retry_format;
				break;

			case 'u':
				if (long_mode) {
					output_str += to_string((u64)va_arg(args, u64));
				} else {
					output_str += to_string((u32)va_arg(args, u32));
				}
				break;

			case 'p':
				long_mode = true;
				output_str += "0x";
				/* FALLTHROUGH */

			case 'x':
				if (long_mode) {
					output_str += to_string((u64)va_arg(args, u64), 16).pad(pad_width, pad_char, pad_side::LEFT);
				} else {
					output_str += to_string((u32)va_arg(args, u32), 16).pad(pad_width, pad_char, pad_side::LEFT);
				}
				break;

			case 'd':
				if (long_mode) {
					output_str += to_string((s64)va_arg(args, s64));
				} else {
					output_str += to_string((s32)va_arg(args, s32));
				}
				break;

			case 'c':
				output_str += (char)va_arg(args, int);
				break;

			case 's':
				output_str += string(va_arg(args, const char *));
				break;

			case '0':
				if (!has_pad) {
					has_pad = true;
					pad_char = '0';
				} else {
					pad_width *= 10;
				}
				goto retry_format;

			case '1' ... '9':
				has_pad = true;
				pad_width *= 10;
				pad_width += *fmt_data - '0';
				goto retry_format;
			}

			break;

		default:
			output_str += *fmt_data;
			break;
		}

		fmt_data++;
	}

	va_end(args);

	return output_str;
}

static string number(u64 v, int base, bool sgn)
{
	u64 current_value = v;

	// Special-case Zero
	if (current_value == 0)
		return "0";

	char buffer[24] = { 0 };
	int i = 22;
	while (current_value > 0 && i-- >= 0) {
		u8 digit = current_value % base;

		char c;
		if (digit < 10)
			c = '0' + digit;
		else
			c = 'a' + (digit - 10);

		buffer[i] = c;
		current_value /= base;
	}

	return string(&buffer[i]);
}

string string::to_string(u32 i) { return number((u64)i, 10, false); }

string string::to_string(s32 i) { return number((u64)(s64)i, 10, true); }

string string::to_string(u64 i) { return number(i, 10, false); }

string string::to_string(s64 i) { return number(i, 10, true); }

string string::to_string(u64 i, int base) { return number(i, base, false); }

list<string> string::split(char delim, bool remove_empty)
{
	list<string> result;

	const char *start = data_;
	const char *end = (data_ + size_);
	string current_part;

	while (start < end) {
		if (*start == delim) {
			if (current_part.length() == 0 && remove_empty) {
				continue;
			}

			result.append(current_part);
			current_part = "";
		} else {
			current_part += *start;
		}

		start++;
	}

	if (current_part.length() > 0 || !remove_empty) {
		result.append(current_part);
	}

	return result;
}
