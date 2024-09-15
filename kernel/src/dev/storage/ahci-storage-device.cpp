/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/ahci-storage-device.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;

device_class ahci_storage_device::ahci_storage_device_class(block_device::block_device_class, "ahci");

void ahci_storage_device::configure()
{
	dprintf("ahci: start port\n");

	// Wait until CR (bit15) is cleared
	while (port_->cmd & HBA_PxCMD_CR)
		;

	// Set FRE (bit4) and ST (bit0)
	port_->cmd |= HBA_PxCMD_FRE;
	port_->cmd |= HBA_PxCMD_ST;

	identify();
}

void ahci_storage_device::identify()
{
	int slot_index;
	volatile hba_cmd_header *cmd = get_free_cmd_slot(slot_index);
	if (cmd == nullptr) {
		panic("no free cmd slots");
	}

	cmd->cfl = sizeof(fis_reg_host2device) / sizeof(u32);
	cmd->w = 0;
	cmd->prdtl = 1;
	cmd->p = 1;

	if (cmd->prdtl > 8) {
		panic("too many prdtls");
	}

	volatile hba_cmd_table *cmdtbl = (hba_cmd_table *)phys_to_virt((u64)cmd->ctba);
	memops::bzero((void *)cmdtbl, sizeof(hba_cmd_table) + sizeof(hba_prdt_entry) * cmd->prdtl);

	u8 *buffer = new u8[512];

	cmdtbl->prdt_entry[0].dba = (u32)(u64)buffer;
	cmdtbl->prdt_entry[0].dbc = 512;
	cmdtbl->prdt_entry[0].i = 1;

	// Prepare command
	volatile fis_reg_host2device *cmdfis = (fis_reg_host2device *)(&cmdtbl->cfis);
	memops::bzero((void *)cmdfis, sizeof(fis_reg_host2device));

	cmdfis->type = fis_type::FIS_TYPE_REG_H2D;
	cmdfis->c = 1;
	cmdfis->command = ATA_CMD_IDENTIFY;

	// Wait for port
	while ((port_->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ))) {
		__relax();
	}

	port_->command_issue = 1 << slot_index; // Issue command

	while (true) {
		if (!(port_->command_issue & (1 << slot_index))) {
			break;
		}

		if (port_->interrupt_status & HBA_PxIS_TFES) // Task file error
		{
			panic("identify error");
		}
	}

	if (port_->interrupt_status & HBA_PxIS_TFES) {
		panic("identify error");
	}

	nr_blocks_ = *(u32 *)(buffer + 120);
	delete[] buffer;
}

void ahci_storage_device::read_blocks_sync(void *buffer, u64 start, u64 count)
{
	int slot_index;
	volatile hba_cmd_header *cmd = get_free_cmd_slot(slot_index);
	if (cmd == nullptr) {
		panic("no free cmd slots");
	}

	cmd->cfl = sizeof(fis_reg_host2device) / sizeof(u32);
	cmd->w = 0;
	cmd->prdtl = (u16)((count - 1) >> 4) + 1;
	cmd->p = 0;

	if (cmd->prdtl > 8) {
		panic("too many prdtls");
	}

	// Prepare buffers
	volatile hba_cmd_table *cmdtbl = (hba_cmd_table *)phys_to_virt((u64)cmd->ctba);
	memops::bzero((void *)cmdtbl, sizeof(hba_cmd_table) + sizeof(hba_prdt_entry) * cmd->prdtl);

	u64 buffer_chunk = (u64)buffer;
	for (int prdt_idx = 0; prdt_idx < cmd->prdtl - 1; prdt_idx++) {
		cmdtbl->prdt_entry[prdt_idx].dba = (u32)buffer_chunk;
		cmdtbl->prdt_entry[prdt_idx].dbc = (8 * 1024) - 1;
		cmdtbl->prdt_entry[prdt_idx].i = 1;

		buffer_chunk += 8 * 1024;
		count -= 16;
	}

	cmdtbl->prdt_entry[cmd->prdtl - 1].dba = (u32)buffer_chunk;
	cmdtbl->prdt_entry[cmd->prdtl - 1].dbc = (count << 9) - 1;
	cmdtbl->prdt_entry[cmd->prdtl - 1].i = 1;

	// Prepare command
	volatile fis_reg_host2device *cmdfis = (fis_reg_host2device *)(&cmdtbl->cfis);
	memops::bzero((void *)cmdfis, sizeof(fis_reg_host2device));

	cmdfis->type = fis_type::FIS_TYPE_REG_H2D;
	cmdfis->c = 1;
	cmdfis->command = ATA_CMD_READ_DMA_EX;

	cmdfis->lba0 = (u8)start;
	cmdfis->lba1 = (u8)(start >> 8);
	cmdfis->lba2 = (u8)(start >> 16);
	cmdfis->lba3 = (u8)(start >> 24);
	cmdfis->lba4 = (u8)(start >> 32);
	cmdfis->lba5 = (u8)(start >> 40);
	cmdfis->device = 1 << 6;

	cmdfis->countl = (u8)count;
	cmdfis->counth = (u8)(count >> 8);

	// Wait for port
	while ((port_->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ))) {
		__relax();
	}

	port_->command_issue = 1 << slot_index; // Issue command

	while (true) {
		if (!(port_->command_issue & (1 << slot_index))) {
			break;
		}

		if (port_->interrupt_status & HBA_PxIS_TFES) // Task file error
		{
			panic("read error");
		}
	}

	if (port_->interrupt_status & HBA_PxIS_TFES) {
		panic("read error");
	}
}

void ahci_storage_device::write_blocks_sync(const void *buffer, u64 start, u64 count) { }

volatile hba_cmd_header *ahci_storage_device::get_free_cmd_slot(int &slot_index)
{
	u32 candidate_slots = port_->sata_ctl | port_->command_issue;
	if (~candidate_slots == 0) {
		return nullptr;
	}

	slot_index = __builtin_ctz(~candidate_slots);
	return &((hba_cmd_header *)phys_to_virt(port_->command_list_base_addr))[slot_index];
}
