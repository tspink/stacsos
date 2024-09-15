/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/device.h>

using namespace stacsos;
using namespace stacsos::kernel::dev;

void device_manager::init() { dprintf("dev: init\n"); }

void device_manager::probe_buses()
{
	for (auto *bus : buses_) {
		bus->probe();
	}
}

string device_manager::register_device(device &device)
{
	string devname = string(device.devclass().name()) + string::to_string(device.devclass().get_next_index());

	dprintf("device-manager: registering device '%s'\n", devname.c_str());

	device.configure();
	devices_.add(devname.get_hash(), &device);

	return devname;
}

void device_manager::add_device_alias(device &device, const string &name) { devices_.add(name.get_hash(), &device); }

bool device_manager::try_get_device_by_class(const device_class &dc, device *&dp)
{
	for (const auto &d : devices_) {
		if (d.value->devclass().is_a(dc)) {
			dp = d.value;
			return true;
		}
	}

	return false;
}

bool device_manager::try_get_device_by_name(const string &name, device *&dp) { return devices_.try_get_value(name.get_hash(), dp); }
