/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/page-allocator.h>

using namespace stacsos::kernel::mem;

void page_allocator::perform_selftest()
{
	dprintf("******************************************\n");
	dprintf("*** PAGE ALLOCATOR SELF TEST ACTIVATED ***\n");
	dprintf("******************************************\n");

	dprintf("(1) Initial state\n");
	dump();

	dprintf("(2) Insert power-of-two block (PFN=0, COUNT=8)\n");
	insert_pages(page::get_from_pfn(0), 8);
	dump();

	dprintf("(3) Insert odd block (PFN=13, COUNT=3)\n");
	insert_pages(page::get_from_pfn(13), 3);
	dump();

	dprintf("(4) Remove page (PFN=2, COUNT=1)\n");
	remove_pages(page::get_from_pfn(2), 1);
	dump();

	dprintf("(5) Allocate page (ORDER=0)\n");
	auto test5page = allocate_pages(0, page_allocation_flags::none);
	if (!test5page) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx, base=%lx\n", test5page->pfn(), test5page->base_address());
	dump();

	dprintf("(6) Free page (PFN=%lx, ORDER=0)\n", test5page->pfn());
	free_pages(*test5page, 0);
	dump();

	dprintf("(7) Insert pages (PFN=20, COUNT=20)\n");
	insert_pages(page::get_from_pfn(20), 20);
	dump();

	dprintf("(8) Allocate page (ORDER=1)\n");
	auto test8page = allocate_pages(1, page_allocation_flags::none);
	if (!test8page) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx, base=%lx\n", test8page->pfn(), test8page->base_address());
	dump();

	dprintf("(9) Free page (PFN=%lx, ORDER=1)\n", test8page->pfn());
	free_pages(*test8page, 1);
	dump();

	dprintf("(10) Insert one page (PFN=2, ORDER=0)\n");
	insert_pages(page::get_from_pfn(2), 1);
	dump();

	dprintf("(11) Allocate page (ORDER=3)\n");
	auto test11page = allocate_pages(3, page_allocation_flags::none);
	if (!test11page) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx, base=%lx\n", test11page->pfn(), test11page->base_address());
	dump();

	auto test12page = test11page + 1;

	dprintf("(12) Free one page in middle of allocation (PFN=%lx, ORDER=0)\n", test12page->pfn());
	free_pages(*test12page, 0);
	dump();

	dprintf("(13) Free one page at start of allocation (PFN=%lx, ORDER=0)\n", test11page->pfn());
	free_pages(*test11page, 0);
	dump();

    dprintf("(14) Insert page to trigger higher merge (PFN=40, ORDER=3)\n");
	insert_pages(page::get_from_pfn(40), 8);
	dump();

	dprintf("*** SELF TEST COMPLETE - SYSTEM TERMINATED ***\n");
	abort();
}
