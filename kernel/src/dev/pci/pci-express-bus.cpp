/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/pci/pci-device-config.h>
#include <stacsos/kernel/dev/pci/pci-device.h>
#include <stacsos/kernel/dev/pci/pci-express-bus.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::pci;
using namespace stacsos;

void pci_express_bus::probe()
{
	dprintf("pcie: probing @ %p\n", base_);
	for (int i = first_; i <= last_; i++) {
		probe_bus(i);
	}
}

void pci_express_bus::probe_bus(unsigned int bus)
{
	for (int slot = 0; slot < 32; slot++) {
		probe_slot(bus, slot);
	}
}

void pci_express_bus::probe_slot(unsigned int bus, unsigned int slot)
{
	probe_func(bus, slot, 0);

	pcie_transport transport(compute_base(0, slot, 0));
	pci_device_configuration config(transport);
	if (config.header_type() & 0x80) {
		for (int func = 1; func < 8; func++) {
			probe_func(bus, slot, func);
		}
	}
}

static const char *pci_device_class_names[] = { "none", "mass storage", "network", "display", "multimedia", "memory", "bridge", "simple comm",
	"base system peripherals", "input", "docking station", "processor", "serial bus", "wireless", "iio", "satellite", "crypto", "signal processing" };

void pci_express_bus::probe_func(unsigned int bus, unsigned int slot, unsigned int func)
{
	pcie_transport *transport = new pcie_transport(compute_base(bus, slot, func));
	pci_device_configuration *config = new pci_device_configuration(*transport);

	// A missing device is not an error.
	if (config->vendor_id() == 0xffff) {
		return;
	}

	dprintf("pcie: %s device @ %u:%u:%u (%04x:%04x)\n", pci_device_class_names[(int)config->class_code()], bus, slot, func, config->vendor_id(),
		config->device_id());

	auto dev = new pci_device( *this, *config);
	device_manager::get().register_device(*dev);
}
