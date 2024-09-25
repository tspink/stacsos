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

    dprintf("  allocated pfn=%lx\n", test5page->pfn());
    dump();

    dprintf("(6) Free page (PFN=%lx, ORDER=0)\n", test5page->pfn());
    free_pages(*test5page, 0);
    dump();

    dprintf("*** SELF TEST COMPLETE - SYSTEM TERMINATED ***\n");
	abort();
}
