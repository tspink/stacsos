/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/cpuid.h>
#include <stacsos/kernel/arch/x86/msr.h>
#include <stacsos/kernel/arch/x86/pit.h>
#include <stacsos/kernel/arch/x86/x2apic.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos;
using namespace stacsos::kernel::arch::x86;

#define MASKED 0x00010000 // Interrupt masked
#define DELIVS 0x00001000 // Delivery status

void x2apic::init()
{
	// Enable x2APIC mode
	u64 apic_base_msr = msrs::ia32_apic_base;
	apic_base_msr |= 0xc00;
	msrs::ia32_apic_base = apic_base_msr;

	// Specify the spurious interrupt vector, and enable the device.
	msr::write(msr_indicies::X2APIC_SVR, 0x1ff);

	// Mask all interrupt sources.
	msrs::x2apic_lvt_timer = MASKED;
	msrs::x2apic_lvt_lint0 = MASKED;
	msrs::x2apic_lvt_lint1 = MASKED;
	msrs::x2apic_lvt_thermal = MASKED;
	msrs::x2apic_lvt_error = MASKED;

	// If the APIC version is >= 4, mask the PCINT interrupt source
	if (((msrs::x2apic_lapic_ver >> 16) & 0xFF) >= 4) {
		msrs::x2apic_lvt_perfmon = MASKED;
	}

	// Clear-out the ESR
	msrs::x2apic_esr = 0;
	msrs::x2apic_esr = 0;

	// Acknowledge any pending interrupts
	msrs::x2apic_eoi = 0;

	// Set-up the interrupt control register
	x2apic_icr i;
	i.trigger_mode = icr_trigger_mode::level;
	i.delivery_mode = icr_delivery_mode::init;
	i.dest_shorthand = 2;

	msrs::x2apic_icr = i.bits;

	// Wait for pending deliveries to complete
	while (msrs::x2apic_icr & DELIVS) {
		asm volatile("");
	}

	msrs::x2apic_tpr = 0;

	calibrate_timer();
}

void x2apic::calibrate_timer()
{
	pit p;

	// Initialise the APIC timer
	set_timer_divide(3);
	set_timer_initial_count(1);
	set_timer_irq(32);
	set_timer_one_shot();
	unmask_interrupts(x2apic_lvts::timer);

	dprintf("x2apic-timer: calibrate...\n");

	// Some useful constants for the calibration
#define FACTOR 1000
#define PIT_FREQUENCY (1193180)
#define CALIBRATION_PERIOD (10)
#define CALIBRATION_TICKS (u16)((PIT_FREQUENCY * CALIBRATION_PERIOD) / FACTOR)

	// Initialise the PIT for one-shot
	p.init_oneshot(CALIBRATION_TICKS); // 10ms

	// Start the PIT
	p.go();

	// Start the APIC timer
	set_timer_initial_count(0xffffffffu);

	// Spin until the PIT expires
	while (!p.expired()) {
		asm volatile("");
	}

	// Stop the APIC timer
	mask_interrupts(x2apic_lvts::timer);

	// Calculate the number of ticks per period (accounting for the LAPIC division)
	u64 ticks_per_period = (0xffffffffu - get_timer_current_count());
	ticks_per_period <<= 4;

	// Determine the LAPIC base frequency
	timer_frequency_ = (ticks_per_period * (FACTOR / CALIBRATION_PERIOD));

	dprintf("x2apic-timer: calibrated frequency=%lu\n", timer_frequency_);
}
