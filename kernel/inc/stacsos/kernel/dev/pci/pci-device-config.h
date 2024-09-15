/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/debug.h>

namespace stacsos::kernel::dev::pci {
using stacsos::kernel::arch::x86::ioports;

enum class pci_native_device_class {
	NONE = 0,
	MASS_STORAGE = 1,
	NETWORK = 2,
	DISPLAY = 3,
	MULTIMEDIA = 4,
	MEMORY = 5,
	BRIDGE = 6,
	SIMPLE_COMM = 7,
	BASE_SYSTEM_PERIPHERALS = 8,
	INPUT = 9,
	DOCKING_STATION = 0xA,
	PROCESSOR = 0xB,
	SERIAL_BUS = 0xC,
	WIRELESS = 0xD,
	IIO = 0xE,
	SATELLITE = 0xF,
	CRYPTO = 0x10,
	SIGNAL_PROCESSING = 0x11,
	NA = 0xFF
};

struct pci_device_status {
	pci_device_status(u16 bits)
		: bits_(bits)
	{
	}

	union {
		struct {
			u16 reserved2 : 3;
			u16 interrupt_status : 1;
			u16 capabilities_list : 1;
			u16 mhz66_capable : 1;
			u16 reserved1 : 1;
			u16 fast_b2b_capable : 1;
			u16 master_data_parity_error : 1;
			u16 devsel_timing : 2;
			u16 signalled_target_abort : 1;
			u16 target_abort : 1;
			u16 master_abort : 1;
			u16 signalled_system_error : 1;
			u16 parity_error : 1;
		};
		u16 bits_;
	};
} __packed;

struct pci_transport {
	virtual ~pci_transport() { }

	virtual u32 read_config_word(u8 offset) const = 0;
	virtual void write_config_word(u8 offset, u32 value) = 0;
};

struct pci_legacy_transport : pci_transport {
	pci_legacy_transport(unsigned int bus, unsigned int slot, unsigned int func)
		: bus_(bus)
		, slot_(slot)
		, func_(func)
	{
	}

	u32 read_config_word(u8 offset) const override
	{
		u32 address = (0x80000000u | (bus_ << 16) | (slot_ << 11) | (func_ << 8) | ((u32)offset & ~3u));

		ioports::pci_config_address::write32(address);
		return ioports::pci_config_data::read32();
	}

	void write_config_word(u8 offset, u32 value) override
	{

		u32 address = (0x80000000u | (bus_ << 16) | (slot_ << 11) | (func_ << 8) | ((u32)offset & ~3u));

		ioports::pci_config_address::write32(address);
		ioports::pci_config_data::write32(value);
	}

private:
	unsigned int bus_, slot_, func_;
};

struct pcie_transport : pci_transport {
	pcie_transport(void *base)
		: base_(base)
	{
	}

	u32 read_config_word(u8 offset) const override { return *(u32 *)((uintptr_t)base_ | (offset & ~3u)); }

	void write_config_word(u8 offset, u32 value) override { }

private:
	void *base_;
};

class pci_device_configuration {
public:
	pci_device_configuration(pci_transport &transport)
		: transport_(transport)
	{
	}

	DELETE_DEFAULT_COPY_AND_MOVE(pci_device_configuration)

	u16 vendor_id() const { return read_config_value<u16>(0); }
	u16 device_id() const { return read_config_value<u16>(2); }

	u16 command() const { return read_config_value<u16>(4); }
	pci_device_status status() const { return pci_device_status(read_config_value<u16>(6)); }

	u8 rev_id() const { return read_config_value<u8>(8); }
	u8 prog_if() const { return read_config_value<u8>(9); }
	u8 subclass() const { return read_config_value<u8>(10); }
	pci_native_device_class class_code() const { return (pci_native_device_class)read_config_value<u8>(11); }

	u8 cache_line_size() const { return read_config_value<u8>(12); }
	u8 latency_timer() const { return read_config_value<u8>(13); }
	u8 header_type() const { return read_config_value<u8>(14); }
	u8 bist() const { return read_config_value<u8>(15); }

	// Header 0
	u32 bar0() const { return read_config_value<u32>(16); }
	u32 bar1() const { return read_config_value<u32>(20); }
	u32 bar2() const { return read_config_value<u32>(24); }
	u32 bar3() const { return read_config_value<u32>(28); }
	u32 bar4() const { return read_config_value<u32>(32); }
	u32 bar5() const { return read_config_value<u32>(36); }

	u32 bar_by_index(int index)
	{
		if (index < 0 || index > 5) {
			return -1;
		}

		return read_config_value<u32>(16 + (index * 4));
	}

	u32 cardbus_cis_ptr() const { return read_config_value<u32>(40); }

	u16 subsystem_id() const { return read_config_value<u16>(46); }
	u16 subsystem_vendor_id() const { return read_config_value<u16>(44); }

	u32 expansion_rom_base_address() const { return read_config_value<u32>(48); }

	u8 capabilities_ptr() const { return read_config_value<u8>(52) & ~3u; }

	u8 interrupt_line() const { return read_config_value<u8>(60); }
	u8 interrupt_pin() const { return read_config_value<u8>(61); }
	void interrupt_line(u8 v) { write_config_value<u8>(60, v); }
	void interrupt_pin(u8 v) { write_config_value<u8>(61, v); }
	u8 min_grant() const { return read_config_value<u8>(62); }
	u8 max_latency() const { return read_config_value<u8>(63); }

	// Header 1
	u8 primary_bus_number() const { return read_config_value<u8>(24); }
	u8 secondary_bus_number() const { return read_config_value<u8>(25); }
	u8 subordinate_bus_number() const { return read_config_value<u8>(26); }
	u8 secondary_latency_timer() const { return read_config_value<u8>(27); }

	u8 io_base() const { return read_config_value<u8>(28); }
	u8 io_limit() const { return read_config_value<u8>(29); }
	u16 secondary_status() const { return read_config_value<u16>(30); }

	u16 memory_base() const { return read_config_value<u16>(32); }
	u16 memory_limit() const { return read_config_value<u16>(34); }

	u16 prefetchable_memory_base() const { return read_config_value<u16>(36); }
	u16 prefetchable_memory_limit() const { return read_config_value<u16>(38); }

	u32 prefetchable_base_upper32() const { return read_config_value<u32>(40); }
	u32 prefetchable_limit_upper32() const { return read_config_value<u32>(44); }

	u16 io_base_upper16() const { return read_config_value<u16>(48); }
	u16 io_limit_upper16() const { return read_config_value<u16>(50); }

	u16 bridge_control() const { return read_config_value<u16>(62); }

	template <typename T> T read_config_value(u8 offset) const
	{
		u32 aligned_offset = offset & ~3u;
		u32 value_word = read_config_word(aligned_offset);

		return (T)(value_word >> (8 * (offset & 3u)));
	}

	template <typename T> void write_config_value(u8 offset, T value)
	{
		u32 aligned_offset = offset & ~3u;
		u32 value_word = read_config_word(aligned_offset);

		dprintf("off=%x, val=%x\n", offset, value);

		u32 mask = (1u << (sizeof(T) * 8)) - 1;
		mask <<= (8 * ((u32)offset & 3u));
		mask = ~mask;

		dprintf("mask=%08x\n", mask);

		dprintf("before %x\n", value_word);
		value_word &= mask;
		dprintf("pre %x\n", value_word);
		value_word |= ((u32)value) << (8 * (offset & 3u));

		dprintf("after %x\n", value_word);

		write_config_word(aligned_offset, value_word);
	}

private:
	pci_transport &transport_;

	u32 read_config_word(u8 offset) const { return transport_.read_config_word(offset); }
	void write_config_word(u8 offset, u32 value) { transport_.write_config_word(offset, value); }
};
} // namespace stacsos::kernel::dev::pci
