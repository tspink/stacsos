/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/x86/ioapic.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/arch/x86/x86-platform.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/acpi/acpi.h>
#include <stacsos/kernel/dev/acpi/descriptors.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/pci/pci-express-bus.h>

#define SIG32(__d, __c, __b, __a) ((u32)(__d) | ((u32)__c << 8) | ((u32)__b << 16) | ((u32)__a << 24))
#define RSDP_SIGNATURE 0x2052545020445352

#define MADT_SIGNATURE SIG32('A', 'P', 'I', 'C')
#define FADT_SIGNATURE SIG32('F', 'A', 'C', 'P')
#define DSDT_SIGNATURE SIG32('D', 'S', 'D', 'T')
#define HPET_SIGNATURE SIG32('H', 'P', 'E', 'T')
#define MCFG_SIGNATURE SIG32('M', 'C', 'F', 'G')

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::acpi;
using namespace stacsos::kernel::dev::pci;
using namespace stacsos::kernel::arch;
using namespace stacsos::kernel::arch::x86;

/**
 * Scans memory for the RSDP by looking for the RSDP signature.  Returns a
 * pointer to the RSDP descriptor, if it's found.
 */
const rsdp_descriptor *ACPI::scan_for_rsdp(uintptr_t start, uintptr_t end)
{
	// Align the starting address.
	uintptr_t cur = start & ~0xf;

	// Loop until we run out of memory, or the RSDP signature was detected.
	while (cur < end) {
		const rsdp_descriptor *candidate = (const rsdp_descriptor *)cur;

		// Check the signature, and if it matches, return the candidate.
		if (candidate->signature == RSDP_SIGNATURE)
			return candidate;

		cur += 16;
	}

	// The RSDP was not found.
	return nullptr;
}

/**
 * Attempts to locate the RSDP in memory.
 */
const rsdp_descriptor *ACPI::locate_rsdp()
{
	u64 ebda_address = (u64)(*(u16 *)phys_to_virt(0x40e)) << 4;
	u64 ebda = (u64)phys_to_virt(ebda_address);

	const rsdp_descriptor *ret = scan_for_rsdp((uintptr_t)ebda, (uintptr_t)ebda + 0x400);
	if (!ret) {
		ret = scan_for_rsdp((uintptr_t)phys_to_virt(0xe0000), (uintptr_t)phys_to_virt(0x100000));
	}

	return ret;
}

/**
 * Performs an ACPI checksum on an arbitrary structure.
 */
bool ACPI::is_structure_valid(const void *structure_base, size_t structure_size)
{
	u8 sum = 0;
	for (unsigned int i = 0; i < structure_size; i++) {
		sum += ((u8 *)structure_base)[i];
	}

	return sum == 0;
}

/**
 * Parses an MADT LAPIC entry.
 */
bool ACPI::parse_madt_lapic(const madt_record_lapic *lapic_record)
{
	dprintf("madt: lapic: id=%u, procid=%u, flags=%x\n", lapic_record->apic_id, lapic_record->acpi_processor_id, lapic_record->flags);

	// bool bootstrap = lapic_record->apic_id == 0;
	core_manager::get().register_core(*new x86_core(lapic_record->acpi_processor_id));

	return true;
}

/**
 * Parses an MADT IOAPIC entry.
 */
bool ACPI::parse_madt_ioapic(const madt_record_ioapic *ioapicr)
{
	dprintf("madt: ioapic: id=%u, addr=%p, gsi-base=%u\n", ioapicr->ioapic_id, ioapicr->ioapic_address, ioapicr->gsi_base);

	x86_platform::get().set_ioapic(new ioapic((void *)phys_to_virt(ioapicr->ioapic_address)));
	x86_platform::get().get_ioapic()->initialise();

	return true;
}

/**
 * Parses an MADT ISO entry.
 */
bool ACPI::parse_madt_iso(const madt_record_interrupt_source_override *iso)
{
	// dprintf("madt: iso: bus=%u, irq=%u, gsi=%u, flags=%x\n", iso->bus_source,
	//		iso->irq_source, iso->gsi, iso->flags);

	return true;
}

/**
 * Parses the HPET
 */
bool ACPI::parse_hpet(const hpet *hpet)
{
	dprintf("acpi: found hpet\n");
	return true;
}

/**
 * Parses the MCFG
 */
bool ACPI::parse_mcfg(const mcfg *mcfg)
{
	dprintf("acpi: found mcfg\n");

	const configuration_space_base_address_allocation *ba = mcfg->base_addresses;

	while ((uintptr_t)ba < ((uintptr_t)mcfg + mcfg->header.length)) {
		dprintf("mcfg: base=%p, seg=%u, st=%u\n", ba->base_address, ba->segment_group_nr, ba->starting_pci_bus);

		device_manager::get().register_bus(*new pci_express_bus(platform_bus_, phys_to_virt(ba->base_address), ba->starting_pci_bus, ba->ending_pci_bus));
		ba++;
	}

	return true;
}

/**
 * Parses the DSDT
 */
bool ACPI::parse_dsdt(const dsdt *dsdt)
{
	if (!is_structure_valid(&dsdt->header, dsdt->header.length)) {
		dprintf("acpi: skipping dsdt (invalid checksum)\n");
		return false;
	}

	if (dsdt->header.signature != DSDT_SIGNATURE) {
		return false;
	}

	return true;

	// aml_parser parser(&dsdt->definition_block[0], dsdt->header.length - sizeof(dsdt->header));
	// return parser.parse();
}

/**
 * Parses the FADT.
 */
bool ACPI::parse_fadt(const fadt *fadt)
{
	if (fadt->header.signature != FADT_SIGNATURE) {
		return false;
	}

	dprintf("acpi: fadt: dsdt=%p\n", fadt->dsdt);
	return parse_dsdt((const dsdt *)phys_to_virt(fadt->dsdt));
}

/**
 * Parses the MADT.
 */
bool ACPI::parse_madt(const madt *madt)
{
	if (madt->header.signature != MADT_SIGNATURE) {
		return false;
	}

	dprintf("acpi: madt: lca=%08x, flags=%x\n", madt->lca, madt->flags);

	const madt_record_header *rhs = &madt->records;
	const madt_record_header *rhe = (const madt_record_header *)((uintptr_t)madt + madt->header.length);

	while (rhs < rhe) {
		switch (rhs->type) {
		case 0:
			if (!parse_madt_lapic((const madt_record_lapic *)rhs)) {
				return false;
			}
			break;

		case 1:
			if (!parse_madt_ioapic((const madt_record_ioapic *)rhs)) {
				return false;
			}
			break;

		case 2:
			if (!parse_madt_iso((const madt_record_interrupt_source_override *)rhs)) {
				return false;
			}
			break;

		default:
			dprintf("acpi: madt: unsupported record type=%u, length=%u\n", rhs->type, rhs->length);
			break;
		}

		rhs = (const madt_record_header *)((uintptr_t)rhs + rhs->length);
	}

	return true;
}

void ACPI::initialise()
{
	rsdp_ = locate_rsdp();
	if (!rsdp_) {
		panic("unable to locate rsdp");
	}

	if (!is_structure_valid(rsdp_, sizeof(*rsdp_))) {
		panic("invalid RSDP structure");
	}

	char oemid[7] = { 0 };
	memops::memcpy(oemid, rsdp_->oem_id, 6);

	dprintf("acpi version=%u oemid=%s rsdt=%p\n", rsdp_->revision, oemid, rsdp_->rsdt_address);

	if (rsdp_->revision > 2) {
		panic("unsupported acpi revision");
	}
}

bool ACPI::probe()
{
	const rsdt_table *rsdt = (const rsdt_table *)phys_to_virt(rsdp_->rsdt_address);
	if (!rsdt) {
		return false;
	}

	if (!is_structure_valid(rsdt, rsdt->header.length)) {
		return false;
	}

	const sdt_header *hdr = nullptr;
	for (unsigned int i = 0; i < (rsdt->header.length - sizeof(rsdt->header)) / sizeof(u32); i++) {
		hdr = (const sdt_header *)phys_to_virt(rsdt->sdt_pointers[i]);

		if (!is_structure_valid(hdr, hdr->length)) {
			dprintf("acpi: skipping table (invalid checksum)\n");
			continue;
		}

		switch (hdr->signature) {
		case MADT_SIGNATURE:
			if (!parse_madt((const madt *)hdr)) {
				return false;
			}

			break;

		case FADT_SIGNATURE:
			if (!parse_fadt((const fadt *)hdr)) {
				return false;
			}

			break;

		case HPET_SIGNATURE:
			if (!parse_hpet((const hpet *)hdr)) {
				return false;
			}

			break;

		case MCFG_SIGNATURE:
			if (!parse_mcfg((const mcfg *)hdr)) {
				return false;
			}

			break;

		default:
			dprintf("acpi: unsupported table: %08x\n", hdr->signature);
			break;
		}
	}

	return true;
}
