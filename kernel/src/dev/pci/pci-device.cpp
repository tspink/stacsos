/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/gfx/qemu-stdvga.h>
#include <stacsos/kernel/dev/pci/pci-device.h>
#include <stacsos/kernel/dev/storage/ahci-controller.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;
using namespace stacsos::kernel::dev::gfx;
using namespace stacsos::kernel::dev::pci;

device_class pci_device::pci_device_class(device_class::root, "pci");

pci_device::pci_device(bus &bus, pci_device_configuration &config)
	: device(pci_device_class, bus)
	, config_(config)
{
}

pci_device::~pci_device() { }

void pci_device::configure()
{
	dprintf("pci: vendor=%x, device=%x\n", config().vendor_id(), config().device_id());

	switch (config().vendor_id()) {
	case 0x1234:
		switch (config().device_id()) {
		case 0x1111: {
			auto *dev = new qemu_stdvga(parent_bus(), *this);
			device_manager::get().register_device(*dev);
			break;
		}

		default:
			dprintf("pci: unknown device\n");
			break;
		}
		break;

	case 0x1af4:
		/*switch (config().device_id()) {
		case 0x1000: {
			auto *tpt = new virtio_pci_transport(*this);
			auto *dev = new virtio_network_device(manager(), parent_bus(), *tpt);
			dev->activate();
			break;
		}
		case 0x1001: {
			auto *tpt = new virtio_pci_transport(*this);
			auto *dev = new virtio_block_device(manager(), parent_bus(), *tpt);
			dev->activate();
			break;
		}
		default:
			dprintf("pci: unknown device\n");
			break;
		}*/

		break;

	case 0x8086:
		switch (config().device_id()) {
		case 0x2922: {
			auto *ahci = new ahci_controller(parent_bus(), *this);
			ahci->probe();
			break;
		}

		default:
			dprintf("pci: unknown device\n");
			break;
		}
		break;

	default:
		dprintf("pci: unknown vendor\n");
		break;
	}
}
