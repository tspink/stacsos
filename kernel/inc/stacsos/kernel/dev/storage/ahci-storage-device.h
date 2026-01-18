/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/storage/ahci-structures.h>
#include <stacsos/kernel/dev/storage/block-device.h>

namespace stacsos::kernel::dev::storage {
class ahci_storage_device : public block_device {
public:
	static device_class ahci_storage_device_class;

	ahci_storage_device(bus &parent, volatile hba_port *port)
		: block_device(ahci_storage_device_class, parent)
		, port_(port)
		, nr_blocks_(0)
	{
	}

	virtual ~ahci_storage_device() { }

	virtual void configure() override;

	virtual u64 nr_blocks() const override { return nr_blocks_; }

	void handle_interrupt();

protected:
	virtual void submit_real_io_request(block_io_request &request) override;

private:
	volatile hba_port *port_;
	u64 nr_blocks_;

	volatile hba_cmd_header *get_free_cmd_slot(int &slot_index);
	void identify();
	void detect_partitions();

	int submit_command(u8 command, u64 lba, u64 count, void *buffer);
	void submit_command_sync(u8 command, u64 lba, u64 count, void *buffer);
};
} // namespace stacsos::kernel::dev::storage
