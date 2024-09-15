/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/console/virtual-console.h>
#include <stacsos/kernel/dev/tty/terminal.h>
#include <stacsos/kernel/fs/file.h>

using namespace stacsos;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::tty;
using namespace stacsos::kernel::dev::console;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::fs;

device_class terminal::terminal_device_class(device_class::root, "tty");

void terminal::configure() { }

void terminal::write_char(unsigned char ch, u8 attr)
{
	if (!ch) {
		return;
	}

	ioports::qemu_debug_out::write8(ch);

	if (attached_vc_) {
		attached_vc_->write_char(ch, attr);
	}
}

void terminal::write(const void *buffer, size_t size)
{
	const u8 *cur = (const u8 *)buffer;
	const u8 *end = cur + size;

	while (cur < end) {
		if (*cur == '\e') {
			cur++;

			current_attr_ = *cur++;
		} else {
			write_char(*cur++, current_attr_);
		}
	}
}

void terminal::read(void *raw_buffer, size_t size)
{
	if (size == 0)
		return;

	u8 *buffer = (u8 *)raw_buffer;
	size_t n = 0;
	while (n < size) {

		buffer[n++] = attached_vc_->read_char();
	}
}

class terminal_file : public file {
public:
	terminal_file(terminal &t)
		: file((u64)-1)
		, t_(t)
	{
	}

	virtual size_t pread(void *buffer, size_t offset, size_t length) override
	{
		t_.read(buffer, length);
		return length;
	}

	virtual size_t pwrite(const void *buffer, size_t offset, size_t length) override
	{
		t_.write(buffer, length);
		return length;
	}

private:
	terminal &t_;
};

shared_ptr<file> terminal::open_as_file() { return shared_ptr<file>(new terminal_file(*this)); }
