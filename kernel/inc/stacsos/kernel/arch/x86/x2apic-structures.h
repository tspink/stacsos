/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {
enum class x2apic_lvts {
	timer,
	pmu,
	thermal,
	lint0,
	lint1,
	error,
};

enum class icr_delivery_mode : u64 { fixed = 0, reserved1 = 1, smi = 2, reserved2 = 3, nmi = 4, init = 5, startup = 6, reserved3 = 7 };
enum class icr_dest_mode : u64 { physical = 0, logical = 1 };
enum class icr_level : u64 { deassert = 0, assert = 1 };
enum class icr_trigger_mode : u64 { edge = 0, level = 1 };
enum class icr_dest_shorthand : u64 { none = 0, self = 1, all_including_self = 2, all_excluding_self = 3 };

struct x2apic_icr {
	x2apic_icr()
		: bits(0)
	{
	}

	union {
		struct {
			// LO
			u64 vector : 8;
			icr_delivery_mode delivery_mode : 3;
			u64 dest_mode : 1;
			u64 reserved1 : 2;
			icr_level level : 1;
			icr_trigger_mode trigger_mode : 1;
			u64 reserved2 : 2;
			u64 dest_shorthand : 2;
			u64 reserved3 : 12;

			// HI
			u64 destination : 32;
		};
		u64 bits;
	};
} __packed;
} // namespace stacsos::kernel::arch::x86
