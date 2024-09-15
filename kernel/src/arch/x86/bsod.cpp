/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::arch::x86;

// NOTE: This whole file is hardcoded because if we've gotten here, we've more
// than likely broken in such a way as NOTHING else works.  So just
// clobber everything.

volatile u16 *vga = (u16 *)0xffff'8000'000b'8000;

static void put_at(int x, int y, int attr, u8 c)
{
	int offset = (y * 80) + x;
	vga[offset] = ((attr << 8) | (int)c);
}

static void write_at(int x, int y, int attr, const char *msg)
{
	int offset = (y * 80) + x;

	while (*msg) {
		vga[offset++] = ((attr << 8) | (*msg++));
	}
}

static char itoa_buffer[64];

static char *itoa(unsigned long int value, int base)
{
	char *ptr = &itoa_buffer[sizeof(itoa_buffer) - 2];
	itoa_buffer[sizeof(itoa_buffer) - 1] = 0;

	while (value) {
		int digit = value % base;
		*ptr-- = digit < 10 ? '0' + digit : 'a' + digit - 10;

		value /= base;
	}

	return ptr + 1;
}

/**
 * Generates a BLUE SCREEN OF DEATH!
 */
void bsod(const char *message, const void *mc)
{
	stacsos::kernel::arch::x86::ioports::console_control::write8(0x0a);
	stacsos::kernel::arch::x86::ioports::console_data::write8(0x20);

	// Draw a pleasant blue background
	for (int i = 0; i < (80 * 25); i++) {
		vga[i] = 0x1700;
	}

	// Fill a white line
	for (int i = 0; i < 80; i++) {
		vga[i] = 0x7000;
	}

	// Display the panic information.
	write_at(2, 0, 0x70, "StACSOS - PANIC!");
	write_at(2, 2, 0x1f, "An error has occurred, and the system has halted.");
	write_at(2, 3, 0x17, "Press CTRL + ALT + DEL to reboot, or quit QEMU.");

	// Draw a fancy box.
	for (int i = 3; i < 77; i++) {
		put_at(i, 5, 0x47, 0xc4); // Top line
		put_at(i, 6, 0x47, 0x00); // Center background
		put_at(i, 7, 0x47, 0xc4); // Bottom line
	}

	put_at(3, 5, 0x47, 0xda); // TL corner
	put_at(76, 5, 0x47, 0xbf); // TR corner
	put_at(3, 7, 0x47, 0xc0); // BL corner
	put_at(76, 7, 0x47, 0xd9); // BR corner

	put_at(3, 6, 0x47, 0xb3); // ML vertical
	put_at(76, 6, 0x47, 0xb3); // MR vertical

	// A cheeky shadow.
	for (int i = 4; i < 78; i++) {
		put_at(i, 8, 0x10, 0xdf);
	}

	put_at(77, 6, 0x10, 0xdb);
	put_at(77, 7, 0x10, 0xdb);

	// The panic message.
	write_at(40 - (stacsos::memops::strlen(message) / 2), 6, 0x4f, message);

	// Backtrace!
	u64 rbp, idx = 0, bty = 9, btx = 1;
	asm volatile("mov %%rbp, %0" : "=r"(rbp));

	while (rbp && idx++ < 32) {
		u64 *stack = (u64 *)rbp;

		write_at(btx, bty, 0x1e, "[  ]");
		write_at(btx + 1, bty, 0x1e, itoa(idx, 10));
		write_at(btx + 5, bty, 0x17, itoa(stack[1], 16));

		if (idx == 16) {
			bty = 9;
			btx = 24;
		} else {
			bty++;
		}

		rbp = stack[0];
	}

	// Machine Context
	if (mc != nullptr) {
		const machine_context *mcontext = (const machine_context *)mc;

		write_at(38, 9, 0x1e, "RAX=");
		write_at(42, 9, 0x17, itoa(mcontext->rax, 16));
		write_at(38, 10, 0x1e, "RCX=");
		write_at(42, 10, 0x17, itoa(mcontext->rcx, 16));
		write_at(38, 11, 0x1e, "RDX=");
		write_at(42, 11, 0x17, itoa(mcontext->rdx, 16));
		write_at(38, 12, 0x1e, "RBX=");
		write_at(42, 12, 0x17, itoa(mcontext->rbx, 16));

		write_at(59, 9, 0x1e, "RSI=");
		write_at(63, 9, 0x17, itoa(mcontext->rsi, 16));
		write_at(59, 10, 0x1e, "RDI=");
		write_at(63, 10, 0x17, itoa(mcontext->rdi, 16));
		write_at(59, 11, 0x1e, "RSP=");
		write_at(63, 11, 0x17, itoa(mcontext->rsp, 16));
		write_at(59, 12, 0x1e, "RBP=");
		write_at(63, 12, 0x17, itoa(mcontext->rbp, 16));

		write_at(38, 14, 0x1e, "RIP=");
		write_at(42, 14, 0x17, itoa(mcontext->rip, 16));

	}

	abort();
}
