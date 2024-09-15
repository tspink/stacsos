/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/ioapic.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::arch::x86::irq;
using namespace stacsos;

#define IOAPIC_ID 0
#define IOAPIC_VER 1

void ioapic::initialise()
{
	u32 ver = read(IOAPIC_VER);
	nr_irqs_ = (u8)(ver >> 16);

	dprintf("ioapic id=%u, ver=%u, nr-irqs=%u\n", read(IOAPIC_ID), ver & 0xff, nr_irqs_);

	redirection_entry blank;
	memops::bzero(&blank, sizeof(blank));

	for (unsigned int i = 0; i < nr_irqs_; i++) {
		store_redir_entry(i, blank);
	}
}

void ioapic::store_redir_entry(u8 irq_index, const redirection_entry &e)
{
	if (irq_index >= nr_irqs_) {
		panic("argument out of range");
	}

	u8 configuration_register = 0x10 + (irq_index * 2);
	write(configuration_register, e.low);
	write(configuration_register + 1, e.high);
}

void ioapic::allocate_physical_irq(u32 phys_irq_nr, x86_core &target_core, irq_handler_fn handler_fn, void *handler_fn_arg)
{
	u8 allocated_irq = target_core.irqmgr().allocate_irq(handler_fn, handler_fn_arg);
	map_physical_irq(phys_irq_nr, target_core, allocated_irq);
}

void ioapic::map_physical_irq(u32 phys_irq_nr, x86_core &target_core, u8 target_irq_nr)
{
	if (phys_irq_nr >= nr_irqs_) {
		panic("argument out of range");
	}

	dprintf("ioapic: map phys=%u, core=%u, target=%u\n", phys_irq_nr, target_core.id(), target_irq_nr);

	redirection_entry re;
	memops::bzero(&re, sizeof(re));

	re.delivery_mode = 0;
	re.delivery_status = 0;
	re.destination = target_core.id();
	re.destination_mode = 0;
	re.mask = 0;
	re.pin_polarity = 0;
	re.remote_irr = 0;
	re.trigger_mode = 0;
	re.vector = target_irq_nr;

	store_redir_entry(phys_irq_nr, re);
}
