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

	dprintf("(2) Test merge propagation for insert_free_pages\n");

	dprintf("  insert start=2, count=2\n");
	insert_free_pages(2, 2);
	dump();

	dprintf("  insert start=1, count=1\n");
	insert_free_pages(1, 1);
	dump();

	dprintf("  insert start=0, count=1\n");
	insert_free_pages(0, 1);
	dump();

	dprintf("  allocate\n");
	allocate_pages(2, page_allocation_flags::none);
	dump();

	dprintf("(3) Insert power-of-two block (PFN=0, COUNT=8)\n");
	insert_free_pages(0, 8);
	dump();

	dprintf("(4) Insert odd block (PFN=13, COUNT=3)\n");
	insert_free_pages(13, 3);
	dump();

	dprintf("(5) Insert another block (PFN=1300, COUNT=7)\n");
	insert_free_pages(1300, 7);
	dump();

	dprintf("(6) Allocate page (ORDER=0)\n");
	auto test5page = allocate_pages(0, page_allocation_flags::none);
	if (test5page.is_error()) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx\n", test5page.get_range_start());
	dump();

	dprintf("(7) Free page (PFN=%lx, ORDER=0)\n", test5page.get_range_start());
	free_pages(test5page.get_range_start(), 0);
	dump();

	dprintf("(8) Insert pages (PFN=20, COUNT=20)\n");
	insert_free_pages(20, 20);
	dump();

	dprintf("(9) Allocate page (ORDER=1)\n");
	auto test8page = allocate_pages(1, page_allocation_flags::none);
	if (test8page.is_error()) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx\n", test8page.get_range_start());
	dump();

	dprintf("(10) Free page (PFN=%lx, ORDER=1)\n", test8page.get_range_start());
	free_pages(test8page.get_range_start(), 1);
	dump();

	dprintf("(11) Insert one page (PFN=2, ORDER=0)\n");
	insert_free_pages(2, 1);
	dump();

	dprintf("(12) Allocate page (ORDER=3)\n");
	auto test11page = allocate_pages(3, page_allocation_flags::none);
	if (test11page.is_error()) {
		panic("page allocation failed during self-test");
	}

	dprintf("  allocated pfn=%lx\n", test11page.get_range_start());
	dump();

	auto test12page = test11page.get_range_start() + 1;

	dprintf("(13) Free one page in middle of allocation (PFN=%lx, ORDER=0)\n", test12page);
	free_pages(test12page, 0);
	dump();

	dprintf("(14) Free one page at start of allocation (PFN=%lx, ORDER=0)\n", test11page.get_range_start());
	free_pages(test11page.get_range_start(), 0);
	dump();

	dprintf("(15) Insert page to trigger higher merge (PFN=40, ORDER=3)\n");
	insert_free_pages(40, 8);
	dump();

	dprintf("(16) Allocate too big\n");
	auto test15page = allocate_pages(8, page_allocation_flags::none);
	if (test15page.is_error()) {
		switch (test15page.get_error()) {
		case page_allocator_error::out_of_memory:
			dprintf("good!! allocation failed with correct error code\n");
			break;

		default:
			dprintf("almost good -- but not quite!! allocation failed with incorrect error code\n");
			break;
		}

	} else {
		dprintf("bad!! allocated pfn=%lx\n", test11page.get_range_start());
	}

	dump();

	dprintf("*** SELF TEST COMPLETE - SYSTEM TERMINATED ***\n");
	abort();
}
