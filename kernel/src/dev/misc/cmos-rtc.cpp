#include <stacsos/kernel/dev/misc/cmos-rtc.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::misc;

device_class cmos_rtc::cmos_rtc_device_class(rtc::rtc_device_class, "cmos-rtc");

rtc_timepoint cmos_rtc::read_timepoint() { panic("NOT IMPLEMENTED"); }
