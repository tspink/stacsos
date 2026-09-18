/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/irq/irq-manager.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/storage/ahci-controller.h>
#include <stacsos/kernel/dev/storage/ahci-storage-device.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/list.h>

using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;
using namespace stacsos::kernel::dev::pci;
using namespace stacsos::kernel::mem;
using namespace stacsos::kernel::arch::x86;

void ahci_controller::ahci_irq_handler(u8 irq, void *ctx, void *arg) { ((ahci_controller *)arg)->handle_interrupt(); }

void ahci_controller::probe()
{
	dprintf("ahci: probing...\n");

	abar_ = (hba_mem *)phys_to_virt(pcidev_.config().bar5());
	if (!abar_) {
		dprintf("ahci: unable to resolve abar\n");
		return;
	}

	abar_->generic_host_cntrol.global_host_control |= 0x80000000; // Enable AHCI mode

	// Register an MSI IRQ handler.
	pcidev_.register_msi(ahci_irq_handler, this);

	// Enable interrupts.
	abar_->generic_host_cntrol.global_host_control |= 2;

	list<volatile hba_port *> usable_ports;

	u32 available_ports = abar_->generic_host_cntrol.ports_implemented;
	for (int port_index = 0; port_index < 32; port_index++) {
		if (available_ports & (1 << port_index)) {
			if (detect_port(&abar_->ports[port_index]) == ahci_port_type::sata) {
				usable_ports.append(&abar_->ports[port_index]);
			}
		}
	}

	// Allocate storage for command list, command table, and FIS.
	u64 cl_size = 0x400 * usable_ports.count();
	u64 ctbl_size = 0x100 * 32 * usable_ports.count();
	u64 fis_size = 0x100 * usable_ports.count();

	u64 clb = memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero).to_page().base_address();
	u64 ctbl = memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero).to_page().base_address();
	u64 fis = memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero).to_page().base_address();

	int port_index = 0;
	for (volatile hba_port *port : usable_ports) {
		u64 clb_offset = clb + (0x400 * port_index);
		u64 ctbl_offset = ctbl + (0x100 * 32 * port_index);
		u64 fis_offset = fis + (0x100 * port_index);

		// Initialise command headers in the CLB for this port.
		for (int cmd_idx = 0; cmd_idx < 32; cmd_idx++) {
			u64 ctbl_cmd_offset = ctbl_offset + (0x100 * cmd_idx);

			volatile hba_cmd_header *hdr = &((hba_cmd_header *)phys_to_virt(clb_offset))[cmd_idx];
			hdr->prdtl = 8;
			hdr->ctba = (u32)ctbl_cmd_offset;
			hdr->ctbau = (u32)(ctbl_cmd_offset >> 32);
		}

		devices_[port_index] = activate_port(port, clb_offset, fis_offset);
		device_manager::get().register_device(*devices_[port_index]);

		port_index++;
	}
}

ahci_port_type ahci_controller::detect_port(volatile hba_port *port)
{
	u32 ssts = port->sata_status;
	u8 det = ssts & 0xf;
	u8 ipm = (ssts >> 8) & 0xf;

	if (det != 3) {
		return ahci_port_type::none;
	}

	if (ipm != 1) {
		return ahci_port_type::none;
	}

	switch (port->signature) {
	case SATA_SIG_ATA:
		return ahci_port_type::sata;
	default:
		return ahci_port_type::other;
	}
}

ahci_storage_device *ahci_controller::activate_port(volatile hba_port *port, u64 clb, u64 fis)
{
	dprintf("ahci: activating port clb=%p, fis=%p\n", clb, fis);

	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;

	while ((port->cmd & HBA_PxCMD_FR) | (port->cmd & HBA_PxCMD_CR)) {
		asm volatile("");
	}

	port->command_list_base_addr = (u32)clb;
	port->command_list_base_addr_hi = (u32)(clb >> 32);
	port->fis_base_addr = (u32)fis;
	port->fis_base_addr_hi = (u32)(fis >> 32);
	port->interrupt_enable = ~0;

	return new ahci_storage_device(*this, port);
}

void ahci_controller::handle_interrupt()
{
	// find out which port interrupted
	// locate completed command
	// do something

	u32 interrupted_ports = abar_->generic_host_cntrol.interrupt_status;
	abar_->generic_host_cntrol.interrupt_status = interrupted_ports;

	for (int i = 0; i < 32; i++) {
		if (interrupted_ports & (1 << i)) {
			devices_[i]->handle_interrupt();
		}
	}

	// abar_->generic_host_cntrol.interrupt_status = abar_->generic_host_cntrol.interrupt_status;
	// dprintf("xx %x\n", abar_->generic_host_cntrol.interrupt_status);

	x86_core::this_core().lapic().eoi();
}
