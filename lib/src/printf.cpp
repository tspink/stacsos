/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/printf.h>

using namespace stacsos;

int stacsos::snprintf(char *buffer, int size, const char *fmt, ...)
{
	int rc;
	va_list args;

	va_start(args, fmt);
	rc = vsnprintf(buffer, size, fmt, args);
	va_end(args);

	return rc;
}

int stacsos::sprintf(char *buffer, const char *fmt, ...)
{
	int rc;
	va_list args;

	va_start(args, fmt);
	rc = vsnprintf(buffer, 0x1000, fmt, args);
	va_end(args);

	return rc;
}

static void prepend_to_buffer(char c, char *buffer, int size)
{
	for (int i = size; i > 0; i--) {
		buffer[i] = buffer[i - 1];
	}

	*buffer = c;
}

static int append_num(char *buffer, int size, u64 value, int base, bool sgn, int pad, char pad_char)
{
	int n = 0;

	if (pad >= size) {
		pad = size - 1;
	}

	if (sgn && (s64)value < 0) {
		prepend_to_buffer('-', buffer, n++);
		value = -value;
	}

	if (value > 0) {
		while (value > 0 && n < size) {
			int digit_value = value % base;

			switch (digit_value) {
			case 0 ... 9:
				prepend_to_buffer('0' + digit_value, buffer, n++);
				break;

			case 10 ... 35:
				prepend_to_buffer('a' + digit_value - 10, buffer, n++);
				break;
			}

			value /= base;
		}
	} else if (n < size) {
		prepend_to_buffer('0', buffer, n++);
		*buffer = '0';
	}
	pad -= n;
	while (pad > 0 && n < size) {
		prepend_to_buffer(pad_char, buffer, n++);
		pad--;
	}

	return n;
}

static int append_str(char *buffer, int size, const char *text, int pad, char pad_char)
{
	int n = 0;

	while (*text && n < size) {
		*buffer = *text;

		buffer++;
		text++;

		n++;
	}

	while (n < size && n < pad) {
		*buffer++ = pad_char;
		n++;
	}

	return n;
}

static int append_guid(char *buffer, int size, const unsigned char *guid)
{
	int count = 0;

	u32 p0 = *(u32 *)(guid);
	u16 p1 = *(u16 *)(guid + 4);
	u16 p2 = *(u16 *)(guid + 6);
	u16 p3 = byte_swap(*(u16 *)(guid + 8));
	u64 p4 = byte_swap((u64) * (u32 *)(guid + 10) | (u64)(*(u16 *)(guid + 14)) << 32) >> 16;

	int rc = append_num(buffer, size, p0, 16, false, 8, '0');
	count += rc;
	buffer += rc;
	size -= rc;

	*buffer++ = '-';
	count++;
	size--;

	rc = append_num(buffer, size, p1, 16, false, 4, '0');
	count += rc;
	buffer += rc;
	size -= rc;

	*buffer++ = '-';
	count++;
	size--;

	rc = append_num(buffer, size, p2, 16, false, 4, '0');
	count += rc;
	buffer += rc;
	size -= rc;

	*buffer++ = '-';
	count++;
	size--;
	rc = append_num(buffer, size, p3, 16, false, 4, '0');
	count += rc;
	buffer += rc;
	size -= rc;

	*buffer++ = '-';
	count++;
	size--;

	rc = append_num(buffer, size, p4, 16, false, 12, '0');
	count += rc;
	buffer += rc;
	size -= rc;

	return count;

	// return append_str(buffer, size, "GUID", 0, '0');
}

int stacsos::vsnprintf(char *buffer_base, int size, const char *fmt_base, va_list args)
{
	const char *fmt = fmt_base;
	char *buffer = buffer_base;

	// Handle a zero-sized buffer.
	if (size == 0) {
		return 0;
	}

	// Do the printing, while we are still consuming format characters, and
	// haven't exceeded 'size'.
	int count = 0;
	while (*fmt != 0 && count < (size - 1)) {
		if (*fmt == '%') {
			int pad_size = 0, rc;
			char pad_char = ' ';
			int number_size = 4;

		retry_format:
			fmt++;

			switch (*fmt) {
			case 0:
				continue;

			case '0':
				if (pad_size > 0) {
					pad_size *= 10;
				} else {
					pad_char = '0';
				}
				goto retry_format;

			case '1' ... '9':
				pad_size *= 10;
				pad_size += *fmt - '0';
				goto retry_format;

			case 'd':
			case 'u': {
				u64 v;

				if (number_size == 8) {
					if (*fmt == 'u') {
						v = (u64)va_arg(args, u64);
					} else {
						v = (u64)va_arg(args, s64);
					}
				} else {
					if (*fmt == 'u') {
						v = (u64)(u32)va_arg(args, u32);
					} else {
						v = (u64)(s64)(s32)va_arg(args, s32);
					}
				}

				rc = append_num(buffer, size - 1 - count, v, 10, *fmt == 'd', pad_size, pad_char);
				count += rc;
				buffer += rc;
				break;
			}

			case 'b':
			case 'x':
			case 'p': {
				unsigned long long int v;

				if (number_size == 8 || *fmt == 'p') {
					v = va_arg(args, unsigned long long int);
				} else {
					v = (unsigned long long int)va_arg(args, unsigned int);
				}

				if (*fmt == 'p') {
					rc = append_str(buffer, size - 1 - count, "0x", 0, ' ');
					count += rc;
					buffer += rc;
				}

				rc = append_num(buffer, size - 1 - count, v, (*fmt == 'b' ? 2 : 16), false, pad_size, pad_char);
				count += rc;
				buffer += rc;
				break;
			}

			case 'l':
				number_size = 8;
				goto retry_format;

			case 's':
				rc = append_str(buffer, size - 1 - count, va_arg(args, const char *), pad_size, pad_char);
				count += rc;
				buffer += rc;
				break;

			case 'c':
				*buffer = va_arg(args, int);

				buffer++;
				count++;
				break;

			case 'G':
				rc = append_guid(buffer, size - 1 - count, va_arg(args, const unsigned char *));
				count += rc;
				buffer += rc;
				break;

			default:
				*buffer = *fmt;

				buffer++;
				count++;
				break;
			}

			fmt++;
		} else {
			*buffer = *fmt;

			buffer++;
			fmt++;
			count++;
		}
	}

	// Null-terminate the buffer
	*buffer = 0;
	return count;
}
