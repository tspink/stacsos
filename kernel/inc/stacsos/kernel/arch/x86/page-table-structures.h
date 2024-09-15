/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/debug.h>

namespace stacsos::kernel::arch::x86 {
struct base_entry {
	u64 bits;

	bool present() const { return get_bit(0); }
	void present(bool v) { update_bit(0, v); }

	bool rw() const { return get_bit(1); }
	void rw(bool v) { update_bit(1, v); }

	bool us() const { return get_bit(2); }
	void us(bool v) { update_bit(2, v); }

	bool pwt() const { return get_bit(3); }
	bool pcd() const { return get_bit(4); }
	bool a() const { return get_bit(5); }

	bool size() const { return get_bit(7); }
	void size(bool v) { update_bit(7, v); }

	bool xd() const { return get_bit(63); }

	u64 base_address() const { return (bits & base_address_mask); }

	void base_address(u64 addr) { bits = (bits & ~base_address_mask) | (addr & base_address_mask); }

	void reset() { bits = 0; }

	void dump() const { dprintf("%016lx", bits); }

private:
	static const u64 base_address_mask = ((1ull << 52) - 1ull) & (~0xfffull);

	void update_bit(int bit, bool value) { bits = (bits & ~(1ull << bit)) | (((u64)(!!value)) << bit); }

	bool get_bit(int bit) const { return !!(bits & (1u << bit)); }

} __packed;

template <typename E> struct base_table {
	E entries[0x200];

	const E &operator[](int index) const { return entries[index]; }
	E &operator[](int index) { return entries[index]; }

	void dump() const;
} __packed;

struct pte : base_entry {
} __packed;

struct pde : base_entry {
} __packed;

struct pdpe : base_entry {
} __packed;

struct pml4e : base_entry {
	u64 pde_base_address() const { return bits & 1; }
} __packed;

using pt = base_table<pte>;
using pd = base_table<pde>;
using pdp = base_table<pdpe>;
using pml4 = base_table<pml4e>;

} // namespace stacsos::kernel::arch::x86
