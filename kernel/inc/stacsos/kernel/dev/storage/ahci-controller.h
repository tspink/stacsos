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

class ahci_storage_device;
class ahci_controller : public bus {
public:
	ahci_controller(bus &parent, pci::pci_device &pcidev)
		: bus(parent)
		, pcidev_(pcidev)
		, abar_(nullptr)
	{
		memops::bzero(devices_, sizeof(devices_));
	}

	virtual void probe() override;

private:
	static void ahci_irq_handler(u8 irq, void *ctx, void *arg);
	ahci_port_type detect_port(volatile hba_port *port);
	ahci_storage_device *activate_port(volatile hba_port *port, u64 clb, u64 fis);
	void handle_interrupt();

	pci::pci_device &pcidev_;
	volatile hba_mem *abar_;
	ahci_storage_device *devices_[32];
};
} // namespace stacsos::kernel::dev::storage
