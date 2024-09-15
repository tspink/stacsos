/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/elf.h>
#include <stacsos/kernel/arch/console-interface.h>
#include <stacsos/kernel/arch/x86/pio.h>
#include <stacsos/kernel/arch/x86/text-console.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/printf.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::arch;
using namespace stacsos::kernel::arch::x86;

extern "C" char _IMAGE_START[], _IMAGE_END[];

static text_console x86_text_console;
static console_interface *console;

void stacsos::kernel::dprintf_init()
{
	x86_text_console.init();
	dprintf_set_console(&x86_text_console);
}

static char dprint_buffer[512];

void stacsos::kernel::dprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(dprint_buffer, sizeof(dprint_buffer), fmt, args);
	va_end(args);

	console->write(dprint_buffer);
}

void stacsos::kernel::dprintf_set_console(console_interface *iface) { console = iface; }

extern void bsod(const char *msg, const void *mcontext);

__noreturn void panic(const char *fmt, ...)
{
	// TODO: Arch Hack
	asm volatile("cli");

	va_list args;
	va_start(args, fmt);
	vsnprintf(dprint_buffer, sizeof(dprint_buffer), fmt, args);
	va_end(args);

	bsod(dprint_buffer, nullptr);

	hang_loop();
}

__noreturn void panic_with_ctx(const void *ctx, const char *fmt, ...)
{
	// TODO: Arch Hack
	asm volatile("cli");

	va_list args;
	va_start(args, fmt);
	vsnprintf(dprint_buffer, sizeof(dprint_buffer), fmt, args);
	va_end(args);

	bsod(dprint_buffer, ctx);

	hang_loop();
}

void debug_helper::parse_image()
{
	/*u64 kernel_image_start = (u64)phys_to_virt((u64)&_IMAGE_START);
	u64 kernel_image_end = (u64)phys_to_virt((u64)&_IMAGE_END);

	dprintf("kernel image start=0x%lx, end=0x%lx, size=0x%lx bytes\n", kernel_image_start, kernel_image_end, kernel_image_end - kernel_image_start);

	const elf_header<32> *kernel_image_header = (const elf_header<32> *)kernel_image_start;

	if (kernel_image_header->e_ident.ei_magic[0] != 0x7f || kernel_image_header->e_ident.ei_magic[1] != 0x45 || kernel_image_header->e_ident.ei_magic[2] != 0x4c
		|| kernel_image_header->e_ident.ei_magic[3] != 0x46) {
		panic("Invalid magic number (%08x) for kernel image ELF header.", *(u32 *)kernel_image_header->e_ident.ei_magic);
	}

	if (kernel_image_header->e_shentsize != 0x28) {
		panic("Invalid section header entry size %d", kernel_image_header->e_shentsize);
	}

	dprintf("%x\n", kernel_image_header->e_shoff);

	for (u16 shidx = 0; shidx < kernel_image_header->e_shnum; shidx++) {
		const elf_sectionheader<32> *sh = &((const elf_sectionheader<32> *)(kernel_image_start + kernel_image_header->e_shoff))[shidx];

		dprintf("  %p %d\n", sh, sh->sh_name);

		if (sh->sh_type == SHT_SYMTAB) {
			dprintf("found symbol table\n");

			const elf_sym<32> *sym = ((const elf_sym<32> *)(kernel_image_start + sh->sh_offset));
			const elf_sym<32> *sym_end = ((const elf_sym<32> *)(kernel_image_start + sh->sh_offset + sh->sh_size));

			while (sym < sym_end) {
				dprintf("symbol\n");
				sym++;
			}
		}
	}*/
}
