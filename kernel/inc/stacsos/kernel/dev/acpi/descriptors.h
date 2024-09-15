/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::dev::acpi {
// Describes an entry in the RSDP
struct rsdp_descriptor {
	u64 signature;
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_address;
} __packed;

// Describes an entry in the RSDT
struct sdt_header {
	u32 signature;
	u32 length;
	u8 revision;
	u8 checksum;
	char oem_id[6];
	char oem_table_id[8];
	u32 oem_revision;
	u32 creator_id;
	u32 creator_revision;
} __packed;

// Describes the RSDT
struct rsdt_table {
	sdt_header header;
	u32 sdt_pointers[0];
} __packed;

struct madt_record_header {
	u8 type, length;
} __packed;

struct madt_record_lapic {
	madt_record_header header;
	u8 acpi_processor_id;
	u8 apic_id;
	u32 flags;
} __packed;

struct madt_record_ioapic {
	madt_record_header header;
	u8 ioapic_id;
	u8 reserved;
	u32 ioapic_address;
	u32 gsi_base;
} __packed;

struct madt_record_interrupt_source_override {
	madt_record_header header;
	u8 bus_source;
	u8 irq_source;
	u32 gsi;
	u16 flags;
} __packed;

struct madt {
	sdt_header header;
	u32 lca;
	u32 flags;
	madt_record_header records; // VARIABLE LENGTH
} __packed;

struct fadt_generic_address {
	u8 address_space;
	u8 bit_width;
	u8 bit_offset;
	u8 access_size;
	u64 address;
} __packed;

struct fadt {
	sdt_header header;
	u32 firmware_ctl;
	u32 dsdt;

	u8 reserved;

	u8 pm_profile;
	u16 sci_interrupt;
	u32 smi_cmdport;
	u8 acpi_enable, acpi_disable;
	u8 s4bios_req;
	u8 pstate_control;

	u32 pm1a_evt_block, pm1b_evt_block, pm1a_ctl_block, pm1b_ctl_block;
	u32 pm2_ctl_block;
	u32 pm_tmr_block;

	u32 gpe0block;
	u32 gpe1block;

	u8 pm1_evt_len;
	u8 pm1_ctl_len;
	u8 pm2_ctl_len;
	u8 pm_tmr_len;

	u8 gpe0_len;
	u8 gpe1_len;
	u8 gpe1_base;

	u8 cstate_ctl;

	u16 worst_c2_latency;
	u16 worst_c3_latency;

	u16 flush_size;
	u16 flush_stride;

	u8 duty_offset;
	u8 duty_width;

	u8 day_alarm;
	u8 month_alarm;
	u8 century;

	u16 boot_arch_flags;

	u8 reserved2;

	u32 flags;

	fadt_generic_address reset_reg;

	u8 reset_value;
	u8 reserved3[3];

	u64 x_firmware_ctl;
	u64 x_dsdt;

	fadt_generic_address x_pm1a_evt_block;
	fadt_generic_address x_pm1b_evt_block;
	fadt_generic_address x_pm1a_ctl_block;
	fadt_generic_address x_pm1b_ctl_block;
	fadt_generic_address x_pm2_ctl_block;
	fadt_generic_address x_pm_tmr_block;
	fadt_generic_address x_gpe0_block;
	fadt_generic_address x_gpe1_block;
} __packed;

struct dsdt {
	sdt_header header;
	u8 definition_block[0];
} __packed;

struct hpet {
	sdt_header header;
	u8 hardware_rev_id;
	u8 comparator_count : 5;
	u8 counter_size : 1;
	u8 reserved : 1;
	u8 legacy_replacement : 1;
	u16 pci_vendor_id;
	fadt_generic_address address;
	u8 hpet_number;
	u16 minimum_tick;
	u8 page_protection;
} __packed;

struct configuration_space_base_address_allocation {
	u64 base_address;
	u16 segment_group_nr;
	u8 starting_pci_bus;
	u8 ending_pci_bus;
	u32 reserved;
} __packed;

struct mcfg {
	sdt_header header;
	u64 reserved;
	configuration_space_base_address_allocation base_addresses[];
} __packed;
} // namespace stacsos::kernel::dev::acpi
