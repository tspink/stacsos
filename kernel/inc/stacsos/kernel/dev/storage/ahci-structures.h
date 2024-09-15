/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::dev::storage {
#define SATA_SIG_ATA 0x00000101

#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000
#define HBA_PxIS_TFES (1u << 30)

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define ATA_CMD_READ_DMA_EX 0xc8
#define ATA_CMD_IDENTIFY 0xec

enum class fis_type : u8 {
	FIS_TYPE_REG_H2D = 0x27, // Register FIS - host to device
	FIS_TYPE_REG_D2H = 0x34, // Register FIS - device to host
	FIS_TYPE_DMA_ACT = 0x39, // DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
	FIS_TYPE_DATA = 0x46, // Data FIS - bidirectional
	FIS_TYPE_BIST = 0x58, // BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS = 0xA1, // Set device bits FIS - device to host
};

struct fis_reg_host2device {
	// DWORD 0
	fis_type type; // FIS_TYPE_REG_H2D

	u8 pmport : 4; // Port multiplier
	u8 rsv0 : 3; // Reserved
	u8 c : 1; // 1: Command, 0: Control

	u8 command; // Command register
	u8 featurel; // Feature register, 7:0

	// DWORD 1
	u8 lba0; // LBA low register, 7:0
	u8 lba1; // LBA mid register, 15:8
	u8 lba2; // LBA high register, 23:16
	u8 device; // Device register

	// DWORD 2
	u8 lba3; // LBA register, 31:24
	u8 lba4; // LBA register, 39:32
	u8 lba5; // LBA register, 47:40
	u8 featureh; // Feature register, 15:8

	// DWORD 3
	u8 countl; // Count register, 7:0
	u8 counth; // Count register, 15:8
	u8 icc; // Isochronous command completion
	u8 control; // Control register

	// DWORD 4
	u8 rsv1[4]; // Reserved
} __packed;

struct hba_cmd_header {
	// DW0
	u8 cfl : 5; // Command FIS length in DWORDS, 2 ~ 16
	u8 a : 1; // ATAPI
	u8 w : 1; // Write, 1: H2D, 0: D2H
	u8 p : 1; // Prefetchable

	u8 r : 1; // Reset
	u8 b : 1; // BIST
	u8 c : 1; // Clear busy upon R_OK
	u8 rsv0 : 1; // Reserved
	u8 pmp : 4; // Port multiplier port

	u16 prdtl; // Physical region descriptor table length in entries

	// DW1
	u32 prdbc; // Physical region descriptor byte count transferred

	// DW2, 3
	u32 ctba; // Command table descriptor base address
	u32 ctbau; // Command table descriptor base address upper 32 bits

	// DW4 - 7
	u32 rsv1[4]; // Reserved
} __packed;

struct hba_port {
	u32 command_list_base_addr;
	u32 command_list_base_addr_hi;
	u32 fis_base_addr;
	u32 fis_base_addr_hi;
	u32 interrupt_status;
	u32 interrupt_enable;
	u32 cmd;
	u32 rsv0;
	u32 task_file_data;
	u32 signature;
	u32 sata_status;
	u32 sata_ctl;
	u32 sata_error;
	u32 sata_active;
	u32 command_issue;
	u32 sata_notification;
	u32 fbs;
	u32 rsv1[11];
	u32 vendor[4];
} __packed;

struct hba_mem {
	struct {
		u32 host_capabilities;
		u32 global_host_control;
		u32 interrupt_status;
		u32 ports_implemented;
		u32 version;
		u32 ccc_ctl;
		u32 ccc_ports;
		u32 em_loc;
		u32 em_ctl;
		u32 host_capabilities_ext;
		u32 bios_handoff_ctl;
	} __packed generic_host_cntrol;

	u8 reserved[0xa0 - 0x2c];

	u8 vendor[0x100 - 0xa0];

	hba_port ports[];
} __packed;

struct hba_prdt_entry {
	u32 dba; // Data base address
	u32 dbau; // Data base address upper 32 bits
	u32 rsv0; // Reserved

	// DW3
	u32 dbc : 22; // Byte count, 4M max
	u32 rsv1 : 9; // Reserved
	u32 i : 1; // Interrupt on completion
} __packed;

struct hba_cmd_table {
	// 0x00
	u8 cfis[64]; // Command FIS

	// 0x40
	u8 acmd[16]; // ATAPI command, 12 or 16 bytes

	// 0x50
	u8 rsv[48]; // Reserved

	// 0x80
	hba_prdt_entry prdt_entry[]; // Physical region descriptor table entries, 0 ~ 65535
} __packed;

} // namespace stacsos::kernel::dev::storage
