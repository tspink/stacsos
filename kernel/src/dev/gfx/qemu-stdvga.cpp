/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/dev/gfx/qemu-stdvga.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::gfx;
using namespace stacsos::kernel;

device_class qemu_stdvga::qemu_stdvga_device_class(graphics::graphics_device_class, "qsvga");

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9
#define VBE_DISPI_INDEX_VIDEO_MEMORY_64K 0xa

#define VBE_DISPI_ID0 0xB0C0
#define VBE_DISPI_ID1 0xB0C1
#define VBE_DISPI_ID2 0xB0C2
#define VBE_DISPI_ID3 0xB0C3
#define VBE_DISPI_ID4 0xB0C4
#define VBE_DISPI_ID5 0xB0C5

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_GETCAPS 0x02
#define VBE_DISPI_8BIT_DAC 0x20
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

void qemu_stdvga::configure()
{
	fb_ = phys_to_virt((uintptr_t)pd_.config().bar0() & ~0xfu);
	mmio_ = phys_to_virt((uintptr_t)pd_.config().bar2() & ~0xfu);

	u16 id = dispi_read(VBE_DISPI_INDEX_ID);
	// u16 mem = dispi_read(VBE_DISPI_INDEX_VIDEO_MEMORY_64K) * 64 * 1024;
	if ((id & 0xfff0) != VBE_DISPI_ID0) {
		dprintf("stdvga: invalid id\n");
	}
}

void qemu_stdvga::set_mode(int width, int height, int bpp)
{
	hw_blank(false);

	dispi_write(VBE_DISPI_INDEX_ENABLE, 0);
	dispi_write(VBE_DISPI_INDEX_BPP, 32);
	dispi_write(VBE_DISPI_INDEX_XRES, width);
	dispi_write(VBE_DISPI_INDEX_YRES, height);
	dispi_write(VBE_DISPI_INDEX_BANK, 0);
	dispi_write(VBE_DISPI_INDEX_VIRT_WIDTH, width);
	dispi_write(VBE_DISPI_INDEX_VIRT_HEIGHT, height);
	dispi_write(VBE_DISPI_INDEX_X_OFFSET, 0);
	dispi_write(VBE_DISPI_INDEX_Y_OFFSET, 0);
	dispi_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

	for (int i = 0; i < (width * height * 4); i++) {
		((u8 *)fb_)[i] = 0;
	}
}

void qemu_stdvga::reset()
{
	hw_blank(false);
	dispi_write(VBE_DISPI_INDEX_ENABLE, 0);
}

void qemu_stdvga::blit(const void *src, size_t size)
{
	if (fb_) {
		hw_blank(true);
		memops::memcpy(fb_, src, size);
		hw_blank(false);
	}
}
