#include <stacsos/kernel/dev/misc/rtc.h>
#include <stacsos/kernel/fs/file.h>

using namespace stacsos;
using namespace stacsos::kernel::fs;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::misc;

device_class rtc::rtc_device_class(device_class::root, "rtc");

/*
 * The structure that is passed to userspace, when the device is read.
 */
struct userspace_tod_buffer {
	u16 seconds, minutes, hours, day_of_month, month, year;
};

/*
 * Implements file operations for this RTC device when opened by userspace.
 */
class rtc_file : public file {
public:
	rtc_file(rtc &rtc_device)
		: file(sizeof(userspace_tod_buffer))
		, rtc_(rtc_device)
	{
	}

	virtual size_t pread(void *buffer, size_t offset, size_t length) override
	{
		// We must be asked to read at least the size of the userspace time-of-day buffer.
		if (length < sizeof(userspace_tod_buffer)) {
			return 0;
		}

		// Ask the actual device for the time.
		rtc_timepoint tp = rtc_.read_timepoint();

		// Copy from the in-kernel timepoint struct into the userspace time-of-day buffer.
		userspace_tod_buffer *rv = (userspace_tod_buffer *)buffer;
		rv->seconds = tp.seconds;
		rv->minutes = tp.minutes;
		rv->hours = tp.hours;
		rv->day_of_month = tp.day_of_month;
		rv->month = tp.month;
		rv->year = tp.year;

		// We'll only ever "read" exactly the size of the userspace TOD buffer.
		return sizeof(userspace_tod_buffer);
	};

	// No writing allowed!
	virtual size_t pwrite(const void *buffer, size_t offset, size_t length) override { return 0; }

private:
	rtc &rtc_;
};

shared_ptr<file> rtc::open_as_file() { return shared_ptr(new rtc_file(*this)); }
