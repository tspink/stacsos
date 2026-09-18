/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/storage/ahci-storage-device.h>
#include <stacsos/kernel/dev/storage/mbr.h>
#include <stacsos/kernel/mem/page-table.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;
using namespace stacsos::kernel::mem;

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
	detect_partitions();
}

void ahci_storage_device::identify()
{
	u8 *buffer = new u8[512];

	submit_command_sync(ATA_CMD_IDENTIFY, 0, 1, buffer);

	nr_blocks_ = *(u32 *)(buffer + 120);
	delete[] buffer;
}

void ahci_storage_device::submit_command_sync(u8 command, u64 lba, u64 count, void *buffer)
{
	int slot = submit_command(command, lba, count, buffer);

	while (true) {
		if (!(port_->command_issue & (1 << slot))) {
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

int ahci_storage_device::submit_command(u8 command, u64 lba, u64 count, void *buffer)
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

	auto buffer_mapping = page_table::current()->get_mapping((u64)buffer);
	if (buffer_mapping.result == mapping_result::unmapped) {
		panic("destination buffer not mapped");
	}

	u64 buffer_chunk = buffer_mapping.address;
	for (int prdt_idx = 0; prdt_idx < cmd->prdtl - 1; prdt_idx++) {
		cmdtbl->prdt_entry[prdt_idx].dba = (u32)buffer_chunk;
		cmdtbl->prdt_entry[prdt_idx].dbau = (u32)((u64)buffer_chunk >> 32);
		cmdtbl->prdt_entry[prdt_idx].dbc = (8 * 1024) - 1;
		cmdtbl->prdt_entry[prdt_idx].i = 1;

		buffer_chunk += 8 * 1024;
		count -= 16;
	}

	cmdtbl->prdt_entry[cmd->prdtl - 1].dba = (u32)buffer_chunk;
	cmdtbl->prdt_entry[cmd->prdtl - 1].dbau = (u32)((u64)buffer_chunk >> 32);
	cmdtbl->prdt_entry[cmd->prdtl - 1].dbc = (count << 9) - 1;
	cmdtbl->prdt_entry[cmd->prdtl - 1].i = 1;

	// Prepare command
	volatile fis_reg_host2device *cmdfis = (fis_reg_host2device *)(&cmdtbl->cfis);
	memops::bzero((void *)cmdfis, sizeof(fis_reg_host2device));

	cmdfis->type = fis_type::FIS_TYPE_REG_H2D;
	cmdfis->c = 1;
	cmdfis->command = command;

	cmdfis->lba0 = (u8)lba;
	cmdfis->lba1 = (u8)(lba >> 8);
	cmdfis->lba2 = (u8)(lba >> 16);
	cmdfis->lba3 = (u8)(lba >> 24);
	cmdfis->lba4 = (u8)(lba >> 32);
	cmdfis->lba5 = (u8)(lba >> 40);
	cmdfis->device = 1 << 6;

	cmdfis->countl = (u8)count;
	cmdfis->counth = (u8)(count >> 8);

	// Wait for port
	while ((port_->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ))) {
		__relax();
	}

	port_->command_issue = 1 << slot_index; // Issue command
	return slot_index;
}

void ahci_storage_device::detect_partitions()
{
	mbr m(*this);
	m.scan();
}

void ahci_storage_device::handle_interrupt()
{
	u32 isr = port_->interrupt_status;
	port_->interrupt_status = isr;

	// TODO: Figure out which command, trigger its completion
}

void ahci_storage_device::submit_real_io_request(block_io_request &request)
{
	if (request.direction == block_io_request_direction::read) {
		submit_command_sync(ATA_CMD_READ_DMA_EX, request.start_block, request.block_count, request.buffer);
		request.completion.signal();
	} else {
		panic("UNIMPLEMENTED BLOCK IO WRITE REQUEST");
	}
}

volatile hba_cmd_header *ahci_storage_device::get_free_cmd_slot(int &slot_index)
{
	u32 candidate_slots = port_->sata_ctl | port_->command_issue;
	if (~candidate_slots == 0) {
		return nullptr;
	}

	slot_index = __builtin_ctz(~candidate_slots);
	return &((hba_cmd_header *)phys_to_virt(port_->command_list_base_addr))[slot_index];
}
