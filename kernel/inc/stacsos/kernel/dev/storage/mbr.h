#pragma once

#include <stacsos/kernel/dev/storage/partitioned-device.h>

namespace stacsos::kernel::dev::storage {
class mbr : public partitioned_device {
public:
	mbr(block_device &owner)
		: partitioned_device(owner)
	{
	}

	virtual ~mbr() { }

	virtual void scan() override;
};
} // namespace stacsos::kernel::dev::storage
