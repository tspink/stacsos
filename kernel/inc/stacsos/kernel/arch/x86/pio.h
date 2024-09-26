/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {
class pio {
public:
	template <u8 port> static void outb(u8 data) { asm volatile("outb %%al, %1" ::"a"(data), "i"(port)); }
	static void outb(u16 port, u8 data) { asm volatile("outb %%al, %%dx" ::"a"(data), "d"(port)); }

	template <u8 port> static void outw(u8 data) { asm volatile("outw %%ax, %1" ::"a"(data), "i"(port)); }
	static void outw(u16 port, u16 data) { asm volatile("outw %%ax, %%dx" ::"a"(data), "d"(port)); }

	template <u8 port> static void outl(u8 data) { asm volatile("outl %%eax, %1" ::"a"(data), "i"(port)); }
	static void outl(u16 port, u32 data) { asm volatile("outl %%eax, %%dx" ::"a"(data), "d"(port)); }

	static u8 inb(int port)
	{
		u8 ret;
		asm volatile("inb %%dx, %%al" : "=a"(ret) : "d"((u16)port));
		return ret;
	}

	static u16 inw(int port)
	{
		u16 ret;
		asm volatile("inw %%dx, %%ax" : "=a"(ret) : "d"((u16)port));
		return ret;
	}

	static u32 inl(int port)
	{
		u32 ret;
		asm volatile("inl %%dx, %%eax" : "=a"(ret) : "d"((u16)port));
		return ret;
	}

	static void insw(int port, uintptr_t buffer, size_t count)
	{
		asm volatile("cld\n\trepnz insw" : "=D"(buffer), "=c"(count) : "d"((u16)port), "0"(buffer), "1"(count) : "memory", "cc");
	}

	static void insl(int port, uintptr_t buffer, size_t count)
	{
		asm volatile("cld\n\trepnz insl" : "=D"(buffer), "=c"(count) : "d"((u16)port), "0"(buffer), "1"(count) : "memory", "cc");
	}
};

template <u16 port, bool eight_bit> class ioport_impl;

template <u16 port> class ioport_impl<port, true> {
public:
	static void write8(u8 b) { pio::outb<port>(b); }
	static void write16(u16 b) { pio::outw<port>(b); }
	static void write32(u32 b) { pio::outl<port>(b); }

	static u8 read8() { return pio::inb(port); }
	static u16 read16() { return pio::inw(port); }
	static u32 read32() { return pio::inl(port); }
};

template <u16 port> class ioport_impl<port, false> {
public:
	static void write8(u8 b) { pio::outb(port, b); }
	static void write16(u16 b) { pio::outw(port, b); }
	static void write32(u32 b) { pio::outl(port, b); }

	static u8 read8() { return pio::inb(port); }
	static u16 read16() { return pio::inw(port); }
	static u32 read32() { return pio::inl(port); }
};

template <u16 port> class ioport : public ioport_impl<port, (port < 0x100)> { };

class ioports {
public:
	using pic1_command = ioport<0x20>;
	using pic1_data = ioport<0x21>;
	using pic2_command = ioport<0xa0>;
	using pic2_data = ioport<0xa1>;

	using cmos_select = ioport<0x70>;
	using cmos_data = ioport<0x71>;

	using keyboard_controller = ioport<0x60>;
	using qemu_debug_out = ioport<0xe9>;
	using console_control = ioport<0x3d4>;
	using console_data = ioport<0x3d5>;
	using pci_config_address = ioport<0xcf8>;
	using pci_config_data = ioport<0xcfc>;
};
} // namespace stacsos::kernel::arch::x86
