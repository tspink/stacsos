/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/x86-page-table.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-table-allocator.h>
#include <stacsos/kernel/mem/page.h>

using namespace stacsos::kernel::mem;
using namespace stacsos::kernel::arch::x86;

static u16 pt_index(u64 address) { return (address >> PAGE_BITS) & 0x1ff; }
static u16 pd_index(u64 address) { return (address >> PAGE_BITS >> 9) & 0x1ff; }
static u16 pdp_index(u64 address) { return (address >> PAGE_BITS >> 9 >> 9) & 0x1ff; }
static u16 pml4_index(u64 address) { return (address >> PAGE_BITS >> 9 >> 9 >> 9) & 0x1ff; }

x86_page_table *x86_page_table::create_empty(page_table_allocator &pta)
{
	page *pml4 = pta.allocate();
	return (x86_page_table *)pml4->base_address_ptr();
}

x86_page_table *x86_page_table::create_linked_copy(page_table_allocator &pta)
{
	page *pml4 = pta.allocate();

	const u64 *source_pml4_entries = (u64 *)&pml4_;
	u64 *new_pml4_entries = (u64 *)pml4->base_address_ptr();

	for (int pml4_index = 0; pml4_index < 0x200; pml4_index++) {
		new_pml4_entries[pml4_index] = source_pml4_entries[pml4_index];
	}

	return (x86_page_table *)new_pml4_entries;
}

void x86_page_table::map(page_table_allocator &pta, u64 virtual_address, u64 physical_address, mapping_flags flags, mapping_size size)
{
	// TODO: assert VA canonical
	bool rw = (flags & mapping_flags::writable) == mapping_flags::writable;
	bool user = (flags & mapping_flags::user_accessable) == mapping_flags::user_accessable;

	pml4e &l4 = pml4_[pml4_index(virtual_address)];
	if (!l4.present()) {
		l4.reset();

		page *l3page = pta.allocate();
		l4.base_address(l3page->base_address());
		l4.present(true);
		l4.rw(rw);
		l4.us(user);
	}

	pdpe &l3 = (*(pdp *)page::get_from_base_address(l4.base_address()).base_address_ptr())[pdp_index(virtual_address)];
	if (size == mapping_size::m1g) {
		if (l3.present() && !l3.size()) {
			panic("overlapping mapping");
		} else {
			l3.reset();
			l3.base_address(physical_address);
			l3.size(true);
			l3.present(true);
			l3.rw(rw);
			l3.us(user);
			return;
		}
	} else {
		if (l3.present()) {
			if (l3.size()) {
				panic("overlapping mapping");
			}
		} else {
			page *l2page = pta.allocate();
			l3.reset();
			l3.base_address(l2page->base_address());
			l3.present(true);
			l3.rw(rw);
			l3.us(user);
		}
	}

	pde &l2 = (*(pd *)page::get_from_base_address(l3.base_address()).base_address_ptr())[pd_index(virtual_address)];
	if (size == mapping_size::m2m) {
		if (l2.present() && !l2.size()) {
			panic("overlapping mapping");
		} else {
			l2.reset();
			l2.base_address(physical_address);
			l2.size(true);
			l2.present(true);
			l2.rw(rw);
			l2.us(user);
			return;
		}
	} else {
		if (l2.present()) {
			if (l2.size()) {
				panic("overlapping mapping");
			}
		} else {
			page *l1page = pta.allocate();
			l2.reset();
			l2.base_address(l1page->base_address());
			l2.present(true);
			l2.rw(rw);
			l2.us(user);
		}
	}

	pte &l1 = (*(pt *)page::get_from_base_address(l2.base_address()).base_address_ptr())[pt_index(virtual_address)];
	l1.reset();
	l1.base_address(physical_address);
	l1.present(true);
	l1.rw(rw);
	l1.us(user);
}

void x86_page_table::dump() const
{
	dprintf("vma @ %p (%p)\n", this, this);
	for (int i = 0; i < 0x200; i++) {
		if (!pml4_[i].present()) {
			continue;
		}

		dprintf("  [%03x]: ", i);
		pml4_[i].dump();

		pdp &pdpt = *(pdp *)page::get_from_base_address(pml4_[i].base_address()).base_address_ptr();
		for (int j = 0; j < 0x200; j++) {
			if (!pdpt[j].present()) {
				continue;
			}

			dprintf("    [%03x]: ", j);
			pdpt[j].dump();

			if (pdpt[j].size()) {
				continue;
			}

			pd &pdt = *(pd *)page::get_from_base_address(pdpt[j].base_address()).base_address_ptr();
			for (int k = 0; k < 0x200; k++) {
				if (!pdt[k].present()) {
					continue;
				}

				dprintf("      [%03x]: ", k);
				pdt[k].dump();

				if (pdt[k].size()) {
					continue;
				}

				pt &ptt = *(pt *)page::get_from_base_address(pdt[k].base_address()).base_address_ptr();
				for (int l = 0; l < 0x200; l++) {
					if (!ptt[l].present()) {
						continue;
					}

					dprintf("        [%03x]: ", l);
					ptt[l].dump();
				}
			}
		}
	}
}
