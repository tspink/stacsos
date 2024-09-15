/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/bus.h>
#include <stacsos/kernel/dev/pci/pci-device.h>
#include <stacsos/kernel/dev/storage/ahci-structures.h>

namespace stacsos::kernel::dev::storage {

enum class ahci_port_type { none, sata, other };

class ahci_controller : public bus {
public:
	ahci_controller(bus &parent, pci::pci_device &pcidev)
		: bus(parent)
		, pcidev_(pcidev)
	{
	}

	virtual void probe() override;

private:
	ahci_port_type detect_port(volatile hba_port *port);
	void activate_port(volatile hba_port *port, u64 clb, u64 fis);
	pci::pci_device &pcidev_;
};
} // namespace stacsos::kernel::dev::storage
