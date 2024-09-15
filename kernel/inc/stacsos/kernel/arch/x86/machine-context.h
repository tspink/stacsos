/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#define NULL_SEGMENT_SELECTOR 0x0
#define KERNEL_CODE_SEGMENT_SELECTOR 0x08
#define KERNEL_DATA_SEGMENT_SELECTOR 0x10
#define USER_CODE_SEGMENT_SELECTOR 0x20
#define USER_DATA_SEGMENT_SELECTOR 0x18
#define TSS_SEGMENT_SELECTOR 0x28

namespace stacsos::kernel::arch::x86 {
struct machine_context {
	u64 gs, fs;
	u64 r15, r14, r13, r12, r11, r10, r9, r8;
	u64 rdi, rsi, rbp, rbx, rdx, rcx, rax;
	u64 extra, rip, cs, rflags, rsp, ss;

	void dump() const;
} __packed;

constexpr static u64 cs_offset = __builtin_offsetof(machine_context, cs);

} // namespace stacsos::kernel::arch::x86
