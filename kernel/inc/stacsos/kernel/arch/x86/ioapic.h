/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once
#include <stacsos/kernel/arch/x86/irq/irq-manager.h>

namespace stacsos::kernel::arch::x86 {
class x2apic;

class ioapic {
public:
	ioapic(void *base_address)
		: base_address_((volatile u32 *)base_address)
		, nr_irqs_(0)
	{
	}

	void initialise();

	void allocate_physical_irq(u32 phys_irq_nr, x86_core &target_core, irq::irq_handler_fn handler_fn, void *handler_fn_arg = nullptr);
	void map_physical_irq(u32 phys_irq_nr, x86_core &target_core, u8 target_irq_nr);

private:
	volatile u32 *base_address_;
	u8 nr_irqs_;

	void write(u32 reg, u32 val)
	{
		base_address_[0] = reg;
		__sync_synchronize();
		base_address_[4] = val;
	}

	u32 read(u32 reg)
	{
		base_address_[0] = reg;
		__sync_synchronize();
		return base_address_[4];
	}

	struct redirection_entry {
		union {
			struct {
				u64 low, high;
			} __packed;

			struct {
				u8 vector;
				u8 delivery_mode : 3;
				u8 destination_mode : 1;
				u8 delivery_status : 1;
				u8 pin_polarity : 1;
				u8 remote_irr : 1;
				u8 trigger_mode : 1;
				u8 mask : 1;
				u8 destination;
			} __packed;
		};
	} __packed;

	void store_redir_entry(u8 irq_index, const redirection_entry &e);
};
} // namespace stacsos::kernel::arch::x86
