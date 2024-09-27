/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/dt.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/memops.h>

using namespace stacsos;
using namespace stacsos::kernel::arch::x86;

void descriptor_table::ensure_caller_is_owner()
{
	if (owner_.id() != x86_core::this_core_id()) {
		panic("not allowed to remotely reload descriptor tables");
	}
}

template <int MAX_NR_GDT_ENTRIES>
global_descriptor_table<MAX_NR_GDT_ENTRIES>::global_descriptor_table(x86_core &owner)
	: descriptor_table(owner)
	, current_(0)
{
}

/**
 * Reloads the Global Descriptor Table after modification of the entries.
 */
template <int MAX_NR_GDT_ENTRIES> void global_descriptor_table<MAX_NR_GDT_ENTRIES>::reload()
{
	ensure_caller_is_owner();

	// Load the address of the GDT into the GDTR
	const gdt_pointer ptr = { .length = (u16)(current_ * 8u), .ptr = gdt_ };
	asm volatile("lgdt %0" ::"m"(ptr) : "memory");

	// Reload segment registers.
	asm volatile("mov %0, %%ds" ::"a"(0x10));
	asm volatile("mov %0, %%es" ::"a"(0x10));
	asm volatile("mov %0, %%fs" ::"a"(0x0));
	asm volatile("mov %0, %%gs" ::"a"(0x0));
	asm volatile("mov %0, %%ss" ::"a"(0x10));

	// TODO: Think about reloading the CS register, which needs to
	// be done with a longjmp.  This was previously done in the assembly
	// start-up code, with the temporary GDT, but it should probably be
	// done with the new GDT.
	//
	// We might even be able to do it with a syscall/sysret.
}

/**
 * Inserts a NULL segment descriptor into the GDT.
 * @return Returns true if the insertion was successful, false otherwise.
 */
template <int MAX_NR_GDT_ENTRIES> bool global_descriptor_table<MAX_NR_GDT_ENTRIES>::add_null()
{
	if (current_ >= MAX_NR_GDT_ENTRIES)
		return false;

	dprintf("gdt: add null @ %x\n", current_ * 8);
	gdt_[current_++] = 0;
	return true;
}

/**
 * Inserts a 64-bit code segment descriptor into the GDT.
 * @return Returns true if the insertion was successful, false otherwise.
 */
template <int MAX_NR_GDT_ENTRIES> bool global_descriptor_table<MAX_NR_GDT_ENTRIES>::add_code_segment(descriptor_privilege_level dpl)
{
	if (current_ >= MAX_NR_GDT_ENTRIES)
		return false;

	code_segment_descriptor desc(dpl);

	dprintf("gdt: add cs @ %x\n", current_ * 8);
	gdt_[current_++] = desc.bits;
	return true;
}

/**
 * Inserts a data segment descriptor into the GDT.
 * @param dpl The privilege level that this descriptor describes.
 * @return Returns true if the insertion was successful, false otherwise.
 */
template <int MAX_NR_GDT_ENTRIES> bool global_descriptor_table<MAX_NR_GDT_ENTRIES>::add_data_segment(descriptor_privilege_level dpl)
{
	if (current_ >= MAX_NR_GDT_ENTRIES)
		return false;

	data_segment_descriptor desc(dpl);

	dprintf("gdt: add ds @ %x\n", current_ * 8);
	gdt_[current_++] = desc.bits;
	return true;
}

template <int MAX_NR_GDT_ENTRIES>
bool global_descriptor_table<MAX_NR_GDT_ENTRIES>::add_call_gate(u16 target_segment, descriptor_privilege_level dpl, u64 target)
{
	if (current_ >= MAX_NR_GDT_ENTRIES)
		return false;

	call_gate_descriptor desc(target_segment, dpl, target);

	dprintf("gdt: add cg @ %x\n", current_ * 8);
	gdt_[current_++] = desc.bits_low;
	gdt_[current_++] = desc.bits_high;
	return true;
}

/**
 * Inserts a TSS descriptor into the GDT.
 * @param ptr The linear address of the TSS structure.
 * @return Returns true if the insertion was successful, false otherwise.
 */
template <int MAX_NR_GDT_ENTRIES> bool global_descriptor_table<MAX_NR_GDT_ENTRIES>::add_tss(task_state_segment &tss)
{
	// Need -1 here, because the TSS descriptor takes up TWO GDT slots.
	if (current_ >= MAX_NR_GDT_ENTRIES - 1)
		return false;

	tss_descriptor desc((void *)&tss.tss_[0], sizeof(tss.tss_));

	dprintf("gdt: add tss @ %x\n", current_ * 8);
	gdt_[current_++] = desc.bits_low;
	gdt_[current_++] = desc.bits_high;

	return true;
}

/**
 * Initialises the Interrupt Descriptor Table.
 * @return Returns true if initialisation was successful, false otherwise.
 */
template <int MAX_NR_IDT_ENTRIES>
interrupt_descriptor_table<MAX_NR_IDT_ENTRIES>::interrupt_descriptor_table(x86_core &owner)
	: descriptor_table(owner)
	, max_index_(0)
{
	// Clear out the IDT
	memops::bzero(idt_, sizeof(idt_));

	initialise_exception_vectors();
}

static void unhandled_exception_handler() { panic("unhandled exception"); }

template <int MAX_NR_IDT_ENTRIES> void interrupt_descriptor_table<MAX_NR_IDT_ENTRIES>::initialise_exception_vectors()
{
	for (int i = 0; i < 256; i++) {
		register_interrupt_gate(i, (uintptr_t)unhandled_exception_handler, 0x8, descriptor_privilege_level::ring0);
	}
}

/**
 * Reloads the IDT register after any changes to the IDT.
 */
template <int MAX_NR_IDT_ENTRIES> void interrupt_descriptor_table<MAX_NR_IDT_ENTRIES>::reload()
{
	ensure_caller_is_owner();

	const idt_pointer ptr = { .length = (u16)((max_index_ + 1) * 16u), .ptr = idt_ };
	asm volatile("lidt %0" ::"m"(ptr));
}

/**
 * Registers an interrupt gate in the IDT, at the given index.
 * @param index The index at which to register the interrupt gate.
 * @param addr The address of the interrupt handling routine.
 * @param seg The code-segment selector for the interrupt handling routine.
 * @param dpl The privilege level at which the interrupt gate can be invoked.
 * @return Returns true if the insertion succeeded, false otherwise.
 */
template <int MAX_NR_IDT_ENTRIES>
bool interrupt_descriptor_table<MAX_NR_IDT_ENTRIES>::register_interrupt_gate(int index, uintptr_t addr, u16 seg, descriptor_privilege_level dpl)
{
	if (index < 0 || index >= MAX_NR_IDT_ENTRIES)
		return false;

	interrupt_gate_descriptor desc(addr, seg, dpl);

	idt_[index].low = desc.bits_low;
	idt_[index].high = desc.bits_high;

	if (index > max_index_) {
		max_index_ = index;
	}

	return true;
}

/**
 * Registers a trap gate in the IDT, at the given index.
 * @param index The index at which to register the trap gate.
 * @param addr The address of the trap handling routine.
 * @param seg The code-segment selector for the trap handling routine.
 * @param dpl The privilege level at which the trap gate can be invoked.
 * @return Returns true if the insertion succeeded, false otherwise.
 */
template <int MAX_NR_IDT_ENTRIES>
bool interrupt_descriptor_table<MAX_NR_IDT_ENTRIES>::register_trap_gate(int index, uintptr_t addr, u16 seg, descriptor_privilege_level dpl)
{
	if (index >= MAX_NR_IDT_ENTRIES)
		return false;

	trap_gate_descriptor desc(addr, seg, dpl);

	idt_[index].low = desc.bits_low;
	idt_[index].high = desc.bits_high;

	if (index > max_index_) {
		max_index_ = index;
	}

	return true;
}

/**
 * Initialises the TSS by loading the task register (TR) with the selector of
 * the TSS descriptor in the GDT.
 * @param sel The selector of the TSS descriptor in the GDT.
 * @return Returns true if initialisation was successful, or false otherwise.
 */
task_state_segment::task_state_segment(x86_core &owner)
	: owner_(owner)
{
	memops::bzero(tss_, sizeof(tss_));
}

void task_state_segment::reload(u16 sel)
{
	if (owner_.id() != x86_core::this_core_id()) {
		panic("not allowed to remotely reload descriptor tables");
	}

	asm volatile("ltr %0" ::"r"(sel));
}

void task_state_segment::set_kernel_stack(uintptr_t stack)
{
	u64 *fields = (u64 *)((uintptr_t)tss_ + 4);
	fields[0] = (u64)stack;
}

template class global_descriptor_table<16>;
template class interrupt_descriptor_table<256>;
