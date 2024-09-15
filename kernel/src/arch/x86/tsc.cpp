/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/pit.h>
#include <stacsos/kernel/arch/x86/tsc.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos::kernel::arch::x86;

void tsc::calibrate()
{
	pit p;

	dprintf("tsc: calibrate...\n");

	// Some useful constants for the calibration
#define FACTOR 1000
#define PIT_FREQUENCY (1193180)
#define CALIBRATION_PERIOD (10)
#define CALIBRATION_TICKS (u16)((PIT_FREQUENCY * CALIBRATION_PERIOD) / FACTOR)

	// Initialise the PIT for one-shot
	p.init_oneshot(CALIBRATION_TICKS); // 10ms

	// Start the PIT
	p.go();

	u64 start = read();

	// Spin until the PIT expires
	while (!p.expired()) {
		asm volatile("");
	}

	u64 end = read();

	dprintf("tsc: start=%lu, end=%lu, delta=%lu\n", start, end, end - start);

	// Calculate the number of ticks per period
	u64 ticks_per_period = end - start;

	// Determine the TSC base frequency
	timer_frequency_ = (ticks_per_period * (FACTOR / CALIBRATION_PERIOD));

	dprintf("tsc: calibrated frequency=%lu\n", timer_frequency_);
}
