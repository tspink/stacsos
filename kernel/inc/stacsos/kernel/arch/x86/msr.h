/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {

enum class msr_indicies : unsigned {
	EFER = 0xc0000080,
	STAR = 0xc0000081,
	LSTAR = 0xc0000082,
	CSTAR = 0xc0000083,
	SFMASK = 0xc0000084,

	FS_BASE = 0xc0000100,
	GS_BASE = 0xc0000101,
	KERNEL_GS_BASE = 0xc0000102,
	IA32_TSC_AUX = 0xc0000103,

	IA32_APIC_BASE = 0x1b,
	IA32_FEATURE_CONTROL = 0x3a,
	IA32_LOCAL_APIC_ID = 0x802,

	// VMX Controls
	IA32_VMX_BASIC = 0x480,
	IA32_VMX_PINBASED_CTLS = 0x481,
	IA32_VMX_PROCBASED_CTLS = 0x482,
	IA32_VMX_EXIT_CTLS = 0x483,
	IA32_VMX_ENTRY_CTLS = 0x484,
	IA32_VMX_MISC = 0x485,
	IA32_VMX_CR0_FIXED0 = 0x486,
	IA32_VMX_CR0_FIXED1 = 0x487,
	IA32_VMX_CR4_FIXED0 = 0x488,
	IA32_VMX_CR4_FIXED1 = 0x489,
	IA32_VMX_VMCS_ENUM = 0x48a,
	IA32_VMX_PROCBASED_CTLS2 = 0x48b,
	IA32_VMX_EPT_VPID_CAP = 0x48c,
	IA32_VMX_TRUE_PINBASED_CTLS = 0x48d,
	IA32_VMX_TRUE_PROCBASED_CTLS = 0x48e,
	IA32_VMX_TRUE_EXIT_CTLS = 0x48f,
	IA32_VMX_TRUE_ENTRY_CTLS = 0x490,
	IA32_VMX_VMFUNC = 0x491,

	// x2APIC Registers
	X2APIC_LAPIC_ID = 0x802,
	X2APIC_LAPIC_VER = 0x803,
	X2APIC_TPR = 0x808,
	X2APIC_PPR = 0x80a,
	X2APIC_EOI = 0x80b,
	X2APIC_LDR = 0x80d,
	X2APIC_SVR = 0x80f,
	X2APIC_ISR0 = 0x810,
	X2APIC_ISR1 = 0x811,
	X2APIC_ISR2 = 0x812,
	X2APIC_ISR3 = 0x813,
	X2APIC_ISR4 = 0x814,
	X2APIC_ISR5 = 0x815,
	X2APIC_ISR6 = 0x816,
	X2APIC_ISR7 = 0x817,
	X2APIC_TMR0 = 0x818,
	X2APIC_TMR1 = 0x819,
	X2APIC_TMR2 = 0x81a,
	X2APIC_TMR3 = 0x81b,
	X2APIC_TMR4 = 0x81c,
	X2APIC_TMR5 = 0x81d,
	X2APIC_TMR6 = 0x81e,
	X2APIC_TMR7 = 0x81f,
	X2APIC_IRR0 = 0x820,
	X2APIC_IRR1 = 0x821,
	X2APIC_IRR2 = 0x822,
	X2APIC_IRR3 = 0x823,
	X2APIC_IRR4 = 0x824,
	X2APIC_IRR5 = 0x825,
	X2APIC_IRR6 = 0x826,
	X2APIC_IRR7 = 0x827,
	X2APIC_ESR = 0x828,
	X2APIC_LVT_CMCI = 0x82f,
	X2APIC_ICR = 0x830,
	X2APIC_LVT_TIMER = 0x832,
	X2APIC_LVT_THERMAL = 0x833,
	X2APIC_LVT_PERFMON = 0x834,
	X2APIC_LVT_LINT0 = 0x835,
	X2APIC_LVT_LINT1 = 0x836,
	X2APIC_LVT_ERROR = 0x837,
	X2APIC_TIMER_ICR = 0x838,
	X2APIC_TIMER_CCR = 0x839,
	X2APIC_TIMER_DCR = 0x83e,
	X2APIC_SELF_IPI = 0x83f
};

template <msr_indicies index> class wrapped2_msr {
public:
	u64 read()
	{
		u32 low, high;

		asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((u32)index));
		return (u64)low | (((u64)high) << 32);
	}

	static void write(u64 msr_value)
	{
		u32 low = msr_value & 0xffffffff;
		u32 high = (msr_value >> 32);

		asm volatile("wrmsr" : : "c"((u32)index), "a"(low), "d"(high));
	}

	static void set(u64 value) { write(read() | value); }

	static void unset(u64 value) { write(read() & ~value); }
};

template <msr_indicies index> class wrapped_msr {
public:
	operator u64() const
	{
		u32 low, high;

		asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((u32)index));
		return (u64)low | (((u64)high) << 32);
	}

	wrapped_msr<index> &operator=(u64 msr_value)
	{
		u32 low = msr_value & 0xffffffff;
		u32 high = (msr_value >> 32);

		asm volatile("wrmsr" : : "c"((u32)index), "a"(low), "d"(high));

		return *this;
	}
};

class msrs {
public:
	static wrapped_msr<msr_indicies::EFER> efer;
	static wrapped_msr<msr_indicies::IA32_APIC_BASE> ia32_apic_base;

	static wrapped_msr<msr_indicies::FS_BASE> fsbase;
	static wrapped_msr<msr_indicies::GS_BASE> gsbase;
	static wrapped_msr<msr_indicies::KERNEL_GS_BASE> kernel_gsbase;
	static wrapped_msr<msr_indicies::IA32_TSC_AUX> ia32_tsc_aux;

	static wrapped_msr<msr_indicies::STAR> ia32_star;
	static wrapped_msr<msr_indicies::LSTAR> ia32_lstar;
	static wrapped_msr<msr_indicies::SFMASK> ia32_fmask;

	static wrapped_msr<msr_indicies::IA32_VMX_BASIC> ia32_vmx_basic;
	static wrapped_msr<msr_indicies::IA32_VMX_PINBASED_CTLS> ia32_vmx_pinbased_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_PROCBASED_CTLS> ia32_vmx_procbased_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_EXIT_CTLS> ia32_vmx_exit_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_ENTRY_CTLS> ia32_vmx_entry_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_MISC> ia32_vmx_misc;
	static wrapped_msr<msr_indicies::IA32_VMX_CR0_FIXED0> ia32_vmx_cr0_fixed0;
	static wrapped_msr<msr_indicies::IA32_VMX_CR0_FIXED1> ia32_vmx_cr0_fixed1;
	static wrapped_msr<msr_indicies::IA32_VMX_CR4_FIXED0> ia32_vmx_cr4_fixed0;
	static wrapped_msr<msr_indicies::IA32_VMX_CR4_FIXED1> ia32_vmx_cr4_fixed1;
	static wrapped_msr<msr_indicies::IA32_VMX_VMCS_ENUM> ia32_vmx_vmcs_enum;
	static wrapped_msr<msr_indicies::IA32_VMX_PROCBASED_CTLS2> ia32_vmx_procbased_ctls2;
	static wrapped_msr<msr_indicies::IA32_VMX_EPT_VPID_CAP> ia32_vmx_ept_vpid_cap;
	static wrapped_msr<msr_indicies::IA32_VMX_TRUE_PINBASED_CTLS> ia32_vmx_true_pinbased_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_TRUE_PROCBASED_CTLS> ia32_vmx_true_procbased_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_TRUE_EXIT_CTLS> ia32_vmx_true_exit_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_TRUE_ENTRY_CTLS> ia32_vmx_true_entry_ctls;
	static wrapped_msr<msr_indicies::IA32_VMX_VMFUNC> ia32_vmx_vmfunc;

	static wrapped_msr<msr_indicies::X2APIC_LAPIC_ID> x2apic_lapic_id;
	static wrapped_msr<msr_indicies::X2APIC_LAPIC_VER> x2apic_lapic_ver;
	static wrapped_msr<msr_indicies::X2APIC_TPR> x2apic_tpr;
	static wrapped_msr<msr_indicies::X2APIC_PPR> x2apic_ppr;
	static wrapped_msr<msr_indicies::X2APIC_EOI> x2apic_eoi;
	static wrapped_msr<msr_indicies::X2APIC_LDR> x2apic_ldr;
	static wrapped_msr<msr_indicies::X2APIC_SVR> x2apic_svr;
	static wrapped_msr<msr_indicies::X2APIC_ISR0> x2apic_isr0;
	static wrapped_msr<msr_indicies::X2APIC_ISR1> x2apic_isr1;
	static wrapped_msr<msr_indicies::X2APIC_ISR2> x2apic_isr2;
	static wrapped_msr<msr_indicies::X2APIC_ISR3> x2apic_isr3;
	static wrapped_msr<msr_indicies::X2APIC_ISR4> x2apic_isr4;
	static wrapped_msr<msr_indicies::X2APIC_ISR5> x2apic_isr5;
	static wrapped_msr<msr_indicies::X2APIC_ISR6> x2apic_isr6;
	static wrapped_msr<msr_indicies::X2APIC_ISR7> x2apic_isr7;
	static wrapped_msr<msr_indicies::X2APIC_TMR0> x2apic_tmr0;
	static wrapped_msr<msr_indicies::X2APIC_TMR1> x2apic_tmr1;
	static wrapped_msr<msr_indicies::X2APIC_TMR2> x2apic_tmr2;
	static wrapped_msr<msr_indicies::X2APIC_TMR3> x2apic_tmr3;
	static wrapped_msr<msr_indicies::X2APIC_TMR4> x2apic_tmr4;
	static wrapped_msr<msr_indicies::X2APIC_TMR5> x2apic_tmr5;
	static wrapped_msr<msr_indicies::X2APIC_TMR6> x2apic_tmr6;
	static wrapped_msr<msr_indicies::X2APIC_TMR7> x2apic_tmr7;
	static wrapped_msr<msr_indicies::X2APIC_IRR0> x2apic_irr0;
	static wrapped_msr<msr_indicies::X2APIC_IRR1> x2apic_irr1;
	static wrapped_msr<msr_indicies::X2APIC_IRR2> x2apic_irr2;
	static wrapped_msr<msr_indicies::X2APIC_IRR3> x2apic_irr3;
	static wrapped_msr<msr_indicies::X2APIC_IRR4> x2apic_irr4;
	static wrapped_msr<msr_indicies::X2APIC_IRR5> x2apic_irr5;
	static wrapped_msr<msr_indicies::X2APIC_IRR6> x2apic_irr6;
	static wrapped_msr<msr_indicies::X2APIC_IRR7> x2apic_irr7;
	static wrapped_msr<msr_indicies::X2APIC_ESR> x2apic_esr;
	static wrapped_msr<msr_indicies::X2APIC_LVT_CMCI> x2apic_lvt_cmci;
	static wrapped_msr<msr_indicies::X2APIC_ICR> x2apic_icr;
	static wrapped_msr<msr_indicies::X2APIC_LVT_TIMER> x2apic_lvt_timer;
	static wrapped_msr<msr_indicies::X2APIC_LVT_THERMAL> x2apic_lvt_thermal;
	static wrapped_msr<msr_indicies::X2APIC_LVT_PERFMON> x2apic_lvt_perfmon;
	static wrapped_msr<msr_indicies::X2APIC_LVT_LINT0> x2apic_lvt_lint0;
	static wrapped_msr<msr_indicies::X2APIC_LVT_LINT1> x2apic_lvt_lint1;
	static wrapped_msr<msr_indicies::X2APIC_LVT_ERROR> x2apic_lvt_error;
	static wrapped_msr<msr_indicies::X2APIC_TIMER_ICR> x2apic_timer_icr;
	static wrapped_msr<msr_indicies::X2APIC_TIMER_CCR> x2apic_timer_ccr;
	static wrapped_msr<msr_indicies::X2APIC_TIMER_DCR> x2apic_timer_dcr;
	static wrapped_msr<msr_indicies::X2APIC_SELF_IPI> x2apic_self_ipi;
};

class msr {
public:
	static void write(msr_indicies msr_id, u64 msr_value)
	{
		u32 low = msr_value & 0xffffffff;
		u32 high = (msr_value >> 32);

		asm volatile("wrmsr" : : "c"((u32)msr_id), "a"(low), "d"(high));
	}

	static u64 read(msr_indicies msr_id)
	{
		u32 low, high;

		asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((u32)msr_id));
		return (u64)low | (((u64)high) << 32);
	}
};
} // namespace stacsos::kernel::arch::x86
