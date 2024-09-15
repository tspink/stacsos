/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>
#include <stacsos/printf.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

void console::init()
{
	console_object_ = object::open("/dev/console");
	if (console_object_ == nullptr) {
		stacsos::syscalls::exit((u64)-1);
		while (1) { }
	}
}

void console::write(const char *msg) { console_object_->write(msg, memops::strlen(msg)); }

void console::writef(const char *msg, ...)
{
	static char buffer[1024];
	va_list args;

	va_start(args, msg);
	vsnprintf(buffer, sizeof(buffer) - 1, msg, args);
	va_end(args);

	write(buffer);
}

char console::read_char()
{
	char ch;
	console_object_->read(&ch, 1);

	return ch;
}
