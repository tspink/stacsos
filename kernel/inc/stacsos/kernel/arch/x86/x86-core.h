/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/arch/x86/dt.h>
#include <stacsos/kernel/arch/x86/irq/irq-manager.h>
#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/arch/x86/tsc.h>
#include <stacsos/kernel/arch/x86/x2apic-timer.h>
#include <stacsos/kernel/arch/x86/x2apic.h>

namespace stacsos::kernel::arch::x86 {
class x86_core : public core {
public:
	explicit x86_core(int id)
		: core(id)
		, gdt_(*this)
		, idt_(*this)
		, tss_(*this)
		, irqs_(idt_)
		, lapic_(*this)
		, timer_(lapic_)
	{
	}

	static int this_core_id() { return core::this_core_id(); }
	static x86_core &this_core() { return (x86_core &)core::this_core(); }

	virtual void init() override;
	virtual bool remote_run() override;

	virtual timer &local_timer() override { return timer_; }

	tsc &local_tsc() { return tsc_; }

	irq::irq_manager<256> &irqmgr() { return irqs_; }
	const irq::irq_manager<256> &irqmgr() const { return irqs_; }

	virtual void set_current_tcb(const tcb *tcb) override;
	virtual tcb *get_current_tcb() override;

	x2apic &lapic() { return lapic_; }
	tsc &timestamp_counter() { return tsc_; }

	void *idt_ptr() const { return idt_.ptr(); }
	void *gdt_ptr() const { return gdt_.ptr(); }
	void *tss_ptr() const { return tss_.ptr(); }

	void dump_regs();

private:
	global_descriptor_table<16> gdt_;
	interrupt_descriptor_table<256> idt_;
	task_state_segment tss_;

	struct {
		u64 task_ptr;
		u64 context_ptr;
		u64 cr3;
		u64 kernel_stack;
	} temporary_tcb;

	irq::irq_manager<256> irqs_;

	x2apic lapic_;
	x2apic_timer timer_;
	tsc tsc_;

	static void exception_handler(u8 irq, void *context, void *arg)
	{
		switch (irq) {
		case 0x0d:
			((x86_core *)arg)->handle_gpf((machine_context *)context);
			break;

		case 0x0e:
			((x86_core *)arg)->handle_page_fault((machine_context *)context);
			break;

		default:
			panic_with_ctx(context, "UNHANDLED EXCEPTION %x", irq);
			break;
		}
	}

	void populate_dt();
	// pfn_t prepare_mpstartup_code();
	// void complete_remote_init();

	void handle_gpf(machine_context *mc);
	void handle_page_fault(machine_context *mc);
};
} // namespace stacsos::kernel::arch::x86
