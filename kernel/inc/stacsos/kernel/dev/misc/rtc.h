#pragma once

#include <stacsos/kernel/dev/device.h>

namespace stacsos::kernel::dev::misc {
struct rtc_timepoint {
	u16 seconds, minutes, hours, day_of_month, month, year;
};

class rtc : public device {
public:
	static device_class rtc_device_class;

	rtc(device_class&dc, bus &owner)
		: device(dc, owner)
	{
	}

	virtual rtc_timepoint read_timepoint() = 0;

	virtual shared_ptr<fs::file> open_as_file() override;
};
} // namespace stacsos::kernel::dev::misc
