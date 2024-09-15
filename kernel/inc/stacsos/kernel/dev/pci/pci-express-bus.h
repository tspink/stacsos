/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/bus.h>

namespace stacsos::kernel::dev::pci {
class pci_express_bus : public bus {
public:
	pci_express_bus(bus &parent, void *base, int first_bus_id, int last_bus_id)
		: bus(parent)
		, base_(base)
		, first_(first_bus_id)
		, last_(last_bus_id)
	{
	}

	virtual void probe() override;

private:
	void *base_;
	int first_, last_;

	void probe_bus(unsigned int bus);
	void probe_slot(unsigned int bus, unsigned int slot);
	void probe_func(unsigned int bus, unsigned int slot, unsigned int func);

	void *compute_base(unsigned int bus, unsigned int slot, unsigned int func)
	{
		return (void *)((uintptr_t)base_ + ((bus - first_) << 20 | slot << 15 | func << 12));
	}
};
} // namespace stacsos::kernel::dev::pci
