/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/gfx/graphics.h>
#include <stacsos/kernel/dev/pci/pci-device.h>

namespace stacsos::kernel::dev::gfx {
class qemu_stdvga : public graphics {
public:
	static device_class qemu_stdvga_device_class;

	qemu_stdvga(bus &owner, pci::pci_device &pd)
		: graphics(qemu_stdvga_device_class, owner)
		, pd_(pd)
		, mmio_(nullptr)
		, fb_(nullptr)
	{
	}

	virtual ~qemu_stdvga() { }

	virtual void configure() override;
	virtual void set_mode(int width, int height, int bpp) override;
	virtual void reset() override;
	virtual void *fb() const { return fb_; }
	virtual void blit(const void *src, size_t size) override;

private:
	pci::pci_device &pd_;
	void *mmio_;
	void *fb_;

	u16 dispi_read(u16 reg)
	{
		int offset = 0x500 + (reg << 1);
		return *(volatile u16 *)((volatile u8 *)mmio_ + offset);
	}

	void dispi_write(u16 reg, u16 val)
	{
		int offset = 0x500 + (reg << 1);
		*(volatile u16 *)((volatile u8 *)mmio_ + offset) = val;
	}

	void hw_blank(bool blank)
	{
		*((volatile u8 *)mmio_ + 0x41a);
		*((volatile u8 *)mmio_ + 0x400) = blank ? 0 : 0x20;
	}
};
} // namespace stacsos::kernel::dev::gfx
