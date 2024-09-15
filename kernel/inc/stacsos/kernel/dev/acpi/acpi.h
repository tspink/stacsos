/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/dev/acpi/descriptors.h>

namespace stacsos::kernel::dev {
class bus;

namespace acpi {
	class ACPI {
	public:
		ACPI(bus &platform)
			: platform_bus_(platform)
		{
			initialise();
		}

		bool probe();

	private:
		void initialise();

		bus &platform_bus_;

		const rsdp_descriptor *rsdp_;

		const rsdp_descriptor *locate_rsdp();
		const rsdp_descriptor *scan_for_rsdp(uintptr_t start, uintptr_t end);

		bool is_structure_valid(const void *structure_base, size_t structure_size);

		bool parse_madt(const madt *madt);
		bool parse_madt_lapic(const madt_record_lapic *lapic);
		bool parse_madt_ioapic(const madt_record_ioapic *ioapic);
		bool parse_madt_iso(const madt_record_interrupt_source_override *iso);

		bool parse_fadt(const fadt *fadt);
		bool parse_dsdt(const dsdt *fadt);
		bool parse_hpet(const hpet *fadt);
		bool parse_mcfg(const mcfg *mcfg);
	};
} // namespace acpi
} // namespace stacsos::kernel::dev
