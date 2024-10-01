/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {

enum class cr0_flags : unsigned long {
	protected_mode_enable = 1u << 0,
	monitor_coprocessor = 1u << 1,
	emulation = 1u << 2,
	task_switched = 1u << 3,
	extension_type = 1u << 4,
	numeric_error = 1u << 5,
	write_protect = 1u << 16,
	alignment_mask = 1u << 18,
	not_write_through = 1u << 29,
	cache_disable = 1u << 30,
	paging = 1u << 31,
};

DEFINE_ENUM_FLAG_OPERATIONS(cr0_flags)

class cr0 {
public:
	static cr0_flags read()
	{
		unsigned long val;
		asm volatile("mov %%cr0, %0" : "=r"(val));

		return (cr0_flags)val;
	}

	static void write(cr0_flags flags) { asm volatile("mov %0, %%cr0" ::"r"((unsigned long)flags)); }
};

class cr2 {
public:
	static unsigned long read()
	{
		unsigned long val;
		asm volatile("mov %%cr2, %0" : "=r"(val));

		return val;
	}
};

class cr3 {
public:
	static unsigned long read()
	{
		unsigned long val;
		asm volatile("mov %%cr3, %0" : "=r"(val));

		return val;
	}

	static void write(unsigned long value) { asm volatile("mov %0, %%cr3" ::"r"(value)); }
};

enum class cr4_flags : unsigned long {
	VME = (1u << 0),
	PVI = (1u << 1),
	TSD = (1u << 2),
	DE = (1u << 3),
	PSE = (1u << 4),
	PAE = (1u << 5),
	MCE = (1u << 6),
	PGE = (1u << 7),
	PCE = (1u << 8),
	OSFXSR = (1u << 9),
	OSXMMEXCPT = (1u << 10),
	UMIP = (1u << 11),
	LA57 = (1u << 12),
	VMXE = (1u << 13),
	SMXE = (1u << 14),
	FSGSBASE = (1u << 16),
	PCIDE = (1u << 17),
	OSXSAVE = (1u << 18),
	SMEP = (1u << 20),
	SMAP = (1u << 21),
	PKE = (1u << 22),
};

DEFINE_ENUM_FLAG_OPERATIONS(cr4_flags)

class cr4 {
public:
	static cr4_flags read()
	{
		unsigned long val;
		asm volatile("mov %%cr4, %0" : "=r"(val));

		return (cr4_flags)val;
	}

	static void write(cr4_flags flags) { asm volatile("mov %0, %%cr4" ::"r"((unsigned long)flags)); }
};

class fsbase {
public:
#ifdef USE_FSGSBASE
	static u64 read()
	{
		u64 fsbase;
		asm volatile("rdfsbase %0" : "=r"(fsbase));
		return fsbase;
	}

	static void write(u64 value) { asm volatile("wrfsbase %0" ::"r"(value)); }
#else
	static u64 read()
	{
		u32 low, high;

		asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((u32)0xc0000100u));
		return (u64)low | (((u64)high) << 32);
	}

	static void write(u64 value)
	{
		u32 low = value & 0xffffffff;
		u32 high = (value >> 32);

		asm volatile("wrmsr" : : "c"(0xc0000100u), "a"(low), "d"(high));
	}
#endif
};

class gsbase {
public:
#ifdef USE_FSGSBASE
	static u64 read()
	{
		u64 gsbase;
		asm volatile("rdgsbase %0" : "=r"(gsbase));
		return gsbase;
	}

	static void write(u64 value) { asm volatile("wrgsbase %0" ::"r"(value)); }
#else
	static u64 read()
	{
		u32 low, high;

		asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((u32)0xc0000101u));
		return (u64)low | (((u64)high) << 32);
	}

	static void write(u64 value)
	{
		u32 low = value & 0xffffffff;
		u32 high = (value >> 32);

		asm volatile("wrmsr" : : "c"(0xc0000101u), "a"(low), "d"(high));
	}
#endif
};
} // namespace stacsos::kernel::arch::x86
