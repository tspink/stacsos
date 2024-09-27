/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {

enum class descriptor_privilege_level {
	ring0 = 0,
	ring1 = 1,
	ring2 = 2,
	ring3 = 3,
};

struct data_segment_descriptor {
	data_segment_descriptor(descriptor_privilege_level dpl)
		: limit_low(0)
		, base_low(0)
		, base_middle(0)
		, A(0)
		, W(1)
		, E(0)
		, ra_0_0(0)
		, ra_1_1(1)
		, DPL((u8)dpl)
		, P(1)
		, limit_high(0)
		, AVL(0)
		, ra_0_2(0)
		, B(1)
		, G(0)
		, base_high(0)
	{
	}

	union {
		u64 bits;

		struct {
			u16 limit_low;
			u16 base_low;
			u8 base_middle;

			u8 A : 1;
			u8 W : 1;
			u8 E : 1;
			u8 ra_0_0 : 1;

			u8 ra_1_1 : 1;

			u8 DPL : 2;
			u8 P : 1;

			u8 limit_high : 4;

			u8 AVL : 1;
			u8 ra_0_2 : 1;
			u8 B : 1;
			u8 G : 1;

			u8 base_high;
		} __packed;
	};
} __packed;

struct code_segment_descriptor {

	code_segment_descriptor(descriptor_privilege_level dpl)
		: ra_0_1(0)
		, ra_0_2(0)
		, A(0)
		, R(1)
		, C(0)
		, ra_1_3(1)
		, ra_1_4(1)
		, DPL((u8)dpl)
		, P(1)
		, ra_0_5(0)
		, AVL(0)
		, L(1)
		, D(0)
		, G(0)
		, ra_0_6(0)
	{
	}

	union {
		u64 bits;

		struct {
			u32 ra_0_1;
			u8 ra_0_2;

			u8 A : 1;
			u8 R : 1;
			u8 C : 1;

			u8 ra_1_3 : 1;
			u8 ra_1_4 : 1;

			u8 DPL : 2;
			u8 P : 1;

			u8 ra_0_5 : 4;

			u8 AVL : 1;
			u8 L : 1;
			u8 D : 1;
			u8 G : 1;

			u8 ra_0_6;
		} __packed;
	};
} __packed;

struct call_gate_descriptor {

	call_gate_descriptor(u16 target_segment, descriptor_privilege_level dpl, u64 target)
		: offset_low(target & 0xffff)
		, segment_selector(target_segment)
		, reserved(0)
		, type(0xc)
		, DPL((u8)dpl)
		, P(1)
		, offset_high((target >> 16) & 0xffff)
		, offset_extra_high((target >> 32) & 0xffffffff)
		, none(0)
	{
	}

	union {
		struct {
			u64 bits_low, bits_high;
		} __packed;

		struct {
			u16 offset_low;
			u16 segment_selector;
			u8 reserved;
			u8 type : 5;
			u8 DPL : 2;
			u8 P : 1;
			u16 offset_high;
			u32 offset_extra_high;
			u32 none;
		} __packed;
	};
} __packed;

struct tss_descriptor {

	tss_descriptor(void *ptr, size_t size)
		: limit_low((size & 0xffff))
		, base_low(((uintptr_t)ptr) & 0xffff)
		, base_middle((((uintptr_t)ptr) >> 16) & 0xff)
		, Type(9)
		, ra_0_0(0)
		, DPL(0)
		, P(1)
		, limit_high((size >> 16) & 0xf)
		, AVL(0)
		, ra_0_1(0)
		, ra_1_2(1)
		, G(0)
		, base_high((((uintptr_t)ptr) >> 24) & 0xff)
		, base_xhigh(((uintptr_t)ptr) >> 32)
	{
	}

	union {
		struct {
			u64 bits_low, bits_high;
		} __packed;

		struct {
			u16 limit_low;
			u16 base_low;
			u8 base_middle;

			u8 Type : 4;
			u8 ra_0_0 : 1;

			u8 DPL : 2;
			u8 P : 1;

			u8 limit_high : 4;

			u8 AVL : 1;
			u8 ra_0_1 : 1;
			u8 ra_1_2 : 1;
			u8 G : 1;

			u8 base_high;

			u32 base_xhigh;
			u32 reserved;
		} __packed;
	};
} __packed;

struct interrupt_gate_descriptor {

	interrupt_gate_descriptor(uintptr_t offset, u16 seg, descriptor_privilege_level dpl)
		: offset_low(offset & 0xffffu)
		, segment_selector(seg)
		, ist(0)
		, ra_1c0_0(0x1c0)
		, DPL((u8)dpl)
		, P(1)
		, offset_mid((offset >> 16) & 0xffffu)
		, offset_high((offset >> 32) & 0xffffffffu)
	{
	}

	union {
		struct {
			u64 bits_low, bits_high;
		} __packed;

		struct {
			u16 offset_low;
			u16 segment_selector;
			u8 ist : 3;
			u16 ra_1c0_0 : 10;
			u8 DPL : 2;
			u8 P : 1;
			u16 offset_mid;
			u32 offset_high;
			u32 ra_0_1;
		} __packed;
	};
} __packed;

struct trap_gate_descriptor {

	trap_gate_descriptor(uintptr_t offset, u16 seg, descriptor_privilege_level dpl)
		: offset_low(offset & 0xffffu)
		, segment_selector(seg)
		, ist(0)
		, ra_1e0_0(0x1e0)
		, DPL((u8)dpl)
		, P(1)
		, offset_mid((offset >> 16) & 0xffffu)
		, offset_high((offset >> 32) & 0xffffffffu)
	{
	}

	union {
		struct {
			u64 bits_low, bits_high;
		} __packed;

		struct {
			u16 offset_low;
			u16 segment_selector;
			u8 ist : 2;
			u16 ra_1e0_0 : 10;
			u8 DPL : 2;
			u8 P : 1;
			u16 offset_mid;
			u32 offset_high;
			u32 ra_0_1;
		} __packed;
	};
} __packed;

struct gdt_pointer {
	u16 length;
	const void *ptr;
} __packed;

struct idt_pointer {
	u16 length;
	const void *ptr;
} __packed;

class x86_core;

class descriptor_table {
public:
	descriptor_table(x86_core &owner)
		: owner_(owner)
	{
	}

	virtual void reload() = 0;

	void ensure_caller_is_owner();

private:
	x86_core &owner_;
};

class task_state_segment;

template <int MAX_NR_GDT_ENTRIES> class global_descriptor_table : public descriptor_table {
public:
	global_descriptor_table(x86_core &owner);

	void reload() override;

	bool add_null();
	bool add_code_segment(descriptor_privilege_level dpl);
	bool add_data_segment(descriptor_privilege_level dpl);
	bool add_tss(task_state_segment &tss);

	bool add_call_gate(u16 target_segment, descriptor_privilege_level dpl, u64 target);

	void *ptr() const { return (void *)&gdt_[0]; }

private:
	u8 current_;
	u64 __aligned(16) gdt_[MAX_NR_GDT_ENTRIES];
};

template <int MAX_NR_IDT_ENTRIES> class interrupt_descriptor_table : public descriptor_table {
public:
	static constexpr int NR_ENTRIES = MAX_NR_IDT_ENTRIES;

	interrupt_descriptor_table(x86_core &owner);

	void reload() override;

	bool register_interrupt_gate(int index, uintptr_t addr, u16 seg, descriptor_privilege_level dpl);
	bool register_trap_gate(int index, uintptr_t addr, u16 seg, descriptor_privilege_level dpl);

	void *ptr() const { return (void *)&idt_[0]; }

private:
	void initialise_exception_vectors();

	u8 max_index_;

	struct {
		u64 low, high;
	} __packed __aligned(16) idt_[MAX_NR_IDT_ENTRIES];
};

class task_state_segment {
	friend class global_descriptor_table<16>;

public:
	task_state_segment(x86_core &owner);

	void reload(u16 sel);

	void set_kernel_stack(uintptr_t stack);

	void *ptr() const { return (void *)&tss_[0]; }

private:
	x86_core &owner_;
	__aligned(16) u64 tss_[26];
};
} // namespace stacsos::kernel::arch::x86
