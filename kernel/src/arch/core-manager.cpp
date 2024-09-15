/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::arch;
using namespace stacsos::kernel::arch::x86;

void core_manager::init() { }

void core_manager::go()
{
	dprintf("cores: init\n");

	// Make sure this core is initialised first.
	core::this_core().status_ = core_status::bootstrap;
	core::this_core().init();

	int this_core_id = core::this_core_id();

	// Bring remote cores online
	for (int i = 0; i < max_cores; i++) {
		// Skip missing cores, or this one.
		if (cores_[i] == nullptr || cores_[i]->id_ == this_core_id) {
			continue;
		}

		dprintf("starting core %d...\n", cores_[i]->id_);
		//cores_[i]->status_ = cores_[i]->remote_run() ? core_status::online : core_status::error;
	}

	// Start this core running
	core::this_core().run();
}

void core_manager::register_core(core &c)
{
	if (c.id_ >= max_cores) {
		panic("core id too big");
	}

	if (cores_[c.id_] != nullptr) {
		panic("core id already registered");
	}

	dprintf("core: register core %u\n", c.id_);
	cores_[c.id_] = &c;
	nr_cores_++;
}
