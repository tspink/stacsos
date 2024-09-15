/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/platform.h>

namespace stacsos::kernel::arch::x86 {
class ioapic;

class x86_platform : public platform {
public:
	DEFINE_SINGLETON(x86_platform);

	x86_platform()
		: ioapic_(nullptr)
	{
	}

	virtual void probe() override;

	ioapic *get_ioapic() const
	{
		if (!ioapic_) {
			panic("no ioapic registered");
		}

		return ioapic_;
	}

	void set_ioapic(ioapic *i) { ioapic_ = i; }

private:
	ioapic *ioapic_;
};
} // namespace stacsos::kernel::arch::x86
