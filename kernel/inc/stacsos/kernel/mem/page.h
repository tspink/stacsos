/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

extern "C" void *_DYNAMIC_DATA_START;

namespace stacsos::kernel::mem {
enum class page_type : u32 { none, reserved, system, allocable };
enum class page_state : u32 { free, allocated };

class memory_manager;
class page_allocator_buddy;
class page_allocator_linear;

class page {
	friend class memory_manager;

public:
	static page &get_from_pfn(u64 pfn) { return get_pagearray()[pfn]; }
	static page &get_from_base_address(u64 base_addr) { return get_pagearray()[base_addr >> PAGE_BITS]; }

	u64 pfn() const { return ((u64)this - (u64)get_pagearray()) / sizeof(page); }
	u64 base_address() const { return pfn() << PAGE_BITS; }
	void *base_address_ptr() const { return (void *)(base_address() + 0xffff'8000'0000'0000ull); }

	u64 refcount() const { return refcount_; }
	void acquire() { refcount_++; }
	bool release() { return !(refcount_--); }

private:
	static page *get_pagearray() { return reinterpret_cast<page *>(&_DYNAMIC_DATA_START); }

	page_type type_;
	page_state state_;
	u64 refcount_;
};
} // namespace stacsos::kernel::mem
