/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/x86-platform.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/acpi/acpi.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/input/keyboard.h>

using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::dev::acpi;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::input;
using namespace stacsos::kernel;

void x86_platform::probe()
{
	auto acpi = new ACPI(*this);
	acpi->probe();

	delete acpi;
}
