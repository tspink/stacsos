/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/bus.h>
#include <stacsos/list.h>
#include <stacsos/map.h>
#include <stacsos/string.h>

namespace stacsos::kernel::dev {
class device;
class device_class;

class device_manager {
	friend class device;

public:
	DEFINE_SINGLETON(device_manager)

	device_manager() { }

	virtual ~device_manager() { }

	void init();

	void register_bus(bus &bus) { buses_.append(&bus); }
	void probe_buses();

	string register_device(device &device);
	void add_device_alias(device &device, const string& name);

	bool try_get_device_by_class(const device_class &cls, device *&ptr);
	bool try_get_device_by_name(const string &name, device *&ptr);

	/*bool try_get_device_by_name(const util::string &name, device *&ptr);

	template <class T> T &get_device_by_name(const util::string &name)
	{
		device *dp;
		if (try_get_device_by_name(name, dp)) {
			return *(T *)dp;
		}

		throw util::system_exception("missing device");
	}*/

	template <class T> T &get_device_by_class(const device_class &cls)
	{
		device *dp;
		if (try_get_device_by_class(cls, dp)) {
			return *(T *)dp;
		}

		panic("missing device");
	}

	bus &sysbus() { return system_bus_; }

private:
	map<u64, device *> devices_;
	list<bus *> buses_;
	system_bus system_bus_;
};
} // namespace stacsos::kernel::dev
