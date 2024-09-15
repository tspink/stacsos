/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/device.h>
#include <stacsos/kernel/dev/pci/pci-device-config.h>

namespace stacsos::kernel::dev::pci {
class pci_bus;

struct pci_capability {
	u8 offset, vendor, length;
} __packed;

struct pci_capabilities_iterator {
	typedef pci_capabilities_iterator self;

	pci_capabilities_iterator(pci_device_configuration &config, int offset)
		: config_(config)
		, offset_(offset)
		, next_(-1)
	{
		populate_header();
	}

	const pci_capability operator*() const { return header_; }

	bool operator==(const self &other) { return other.offset_ == offset_; }
	bool operator!=(const self &other) { return other.offset_ != offset_; }

	self &operator++()
	{
		offset_ = next_;
		if (offset_ == 0) {
			offset_ = -1;
		} else {
			populate_header();
		}

		return *this;
	}

private:
	pci_device_configuration &config_;
	unsigned int offset_, next_;
	pci_capability header_;

	void populate_header()
	{
		next_ = config_.read_config_value<u8>(offset_ + 1);

		header_.offset = offset_;
		header_.vendor = config_.read_config_value<u8>(offset_);
		header_.length = config_.read_config_value<u8>(offset_ + 2);
	}
};

struct pci_capabilities_iterable {
	pci_capabilities_iterable(pci_device_configuration &config)
		: config_(config)
	{
	}

	pci_capabilities_iterator begin()
	{
		if (!config_.status().capabilities_list) {
			return pci_capabilities_iterator(config_, -1);
		}

		return pci_capabilities_iterator(config_, config_.capabilities_ptr());
	}

	pci_capabilities_iterator end() { return pci_capabilities_iterator(config_, -1); }

private:
	pci_device_configuration &config_;
};

class pci_device : public device {
public:
	static device_class pci_device_class;

	pci_device(bus &owner, pci_device_configuration &config);
	virtual ~pci_device();

	virtual void configure() override;

	pci_capabilities_iterable capabilities() { return pci_capabilities_iterable(config_); }

	pci_device_configuration &config() const { return config_; }

private:
	pci_device_configuration &config_;
};
} // namespace stacsos::kernel::dev::pci
