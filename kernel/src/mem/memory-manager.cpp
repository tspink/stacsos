/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/config.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-allocator-buddy.h>
#include <stacsos/kernel/mem/page-allocator-linear.h>
#include <stacsos/kernel/mem/page.h>

extern "C" const char *_IMAGE_START;
extern "C" const char *_IMAGE_END;

using namespace stacsos::kernel;
using namespace stacsos::kernel::mem;

struct memory_block {
	u64 start, length;
	bool avail;
};

static memory_block memory_blocks[16];
static int nr_memory_blocks;

static char page_allocator_structure[0x1000];

void memory_manager::init()
{
	dprintf("mem: init\n");

	const char *pgalloc_algorithm_name = config::get().get_option_or_default("pgalloc", "linear");
	dprintf("\e\x04mem: *** using the '%s' page allocator\e\x07\n", pgalloc_algorithm_name);

	void *page_allocator_object = (void *)page_allocator_structure;
	if (memops::strcmp(pgalloc_algorithm_name, "buddy") == 0) {
		pgalloc_ = new (page_allocator_object) page_allocator_buddy(*this);
	} else if (memops::strcmp(pgalloc_algorithm_name, "linear") == 0) {
		pgalloc_ = new (page_allocator_object) page_allocator_linear(*this);
	} else {
		panic("Invalid page allocator algoritm: %s", pgalloc_algorithm_name);
	}

	dprintf("memory:\n");
	u64 last_addr = 0;
	for (int i = 0; i < nr_memory_blocks; i++) {
		const memory_block *mb = &memory_blocks[i];
		dprintf("  %016lx -- %016lx (%S) %%c\n", memory_blocks[i].start, memory_blocks[i].start + memory_blocks[i].length, memory_blocks[i].length,
			mb->avail ? 'A' : 'X');

		u64 block_last_addr = mb->start + mb->length - 1;
		if (block_last_addr > last_addr) {
			last_addr = block_last_addr;
		}
	}

	u64 nr_page_descriptors = (last_addr + 1) >> PAGE_BITS;
	initialise_page_descriptors(nr_page_descriptors);
	initialise_page_allocator(nr_page_descriptors);
	initialise_object_allocator();

	dprintf("switching to primary page table mapping...\n");
	activate_primary_mapping();

	dprintf("done\n");
}

void memory_manager::add_memory_block(u64 start, u64 length, bool avail)
{
	memory_blocks[nr_memory_blocks].start = start;
	memory_blocks[nr_memory_blocks].length = length;
	memory_blocks[nr_memory_blocks].avail = avail;
	nr_memory_blocks++;
}

void memory_manager::initialise_page_descriptors(u64 nr_page_descriptors)
{
	// Indicate to the user how many page descriptors have been detected.
	dprintf("%lu pages (%lu Mb)\n", nr_page_descriptors, (nr_page_descriptors << PAGE_BITS) / 1048576);

	// Initialise all page descriptors to zero.
	memops::bzero(page::get_pagearray(), sizeof(page) * nr_page_descriptors);
}

void memory_manager::initialise_page_allocator(u64 nr_page_descriptors)
{
	if (memops::strcmp(config::get().get_option_or_default("pgalloc-selftest", "no"), "yes") == 0) {
		pgalloc_->perform_selftest();
		__unreachable();
	}

	// Loop through each memory block that we received from the
	// startup code.
	for (int i = 0; i < nr_memory_blocks; i++) {
		const memory_block *mb = &memory_blocks[i];

		// If the memory block is available, then insert them into the page allocator.
		if (mb->avail) {
			// Add these pages to the page allocator
			pgalloc_->insert_pages(page::get_from_pfn(mb->start >> PAGE_BITS), mb->length >> PAGE_BITS);
		} else {
			// Otherwise, mark these pages as reserved.
			for (u64 pfn = (mb->start >> PAGE_BITS); pfn < ((mb->start + mb->length) >> PAGE_BITS); pfn++) {
				auto &pg = page::get_from_pfn(pfn);
				pg.type_ = page_type::reserved;
			}
		}
	}

	// Now we've added all of the "available" memory regions, we need to take out areas
	// that we know are already allocated, e.g. the kernel image.

	// Remove the first megabyte of memory.  Not strictly necessary, but there are
	// various BIOS structures (and the zero page) which we should try to avoid.
	pgalloc_->remove_pages(page::get_from_pfn(0), MB(1) >> PAGE_BITS);

	// Remove early page tables (we'll put them back later, maybe), and the page containing
	// the instantiated page allocator.
	pgalloc_->remove_pages(page::get_from_base_address(0x100000), 6);

	// Remove the hard-coded page allocator structure

	// Remove the kernel image
	u64 image_size = ((u64)&_IMAGE_END) - ((u64)&_IMAGE_START);
	pgalloc_->remove_pages(page::get_from_base_address((u64)&_IMAGE_START), PAGE_ALIGN_UP(image_size) >> PAGE_BITS);

	// Remove the page descriptors array
	u64 page_descriptors_size = sizeof(page) * nr_page_descriptors;
	pgalloc_->remove_pages(page::get_from_base_address((u64)&_DYNAMIC_DATA_START - 0xffff'ffff'8000'0000), PAGE_ALIGN_UP(page_descriptors_size) >> PAGE_BITS);
}

void memory_manager::initialise_object_allocator()
{
	// Nothing to do to initialise the object allocator!
}

void memory_manager::activate_primary_mapping()
{
	root_address_space_ = new address_space(ptalloc_, (u64)0);

	// Insert a mapping that allows us to access physical memory 1-1
	root_address_space_->pgtable().map(ptalloc_, 0xffff'8000'0000'0000, GB(0), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
	root_address_space_->pgtable().map(ptalloc_, 0xffff'8000'4000'0000, GB(1), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
	root_address_space_->pgtable().map(ptalloc_, 0xffff'8000'8000'0000, GB(2), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
	root_address_space_->pgtable().map(ptalloc_, 0xffff'8000'c000'0000, GB(3), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);

	// This mapping is for the kernel high address space.  It's used mainly for executing kernel code.
	root_address_space_->pgtable().map(ptalloc_, 0xffff'ffff'8000'0000, GB(0), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
	root_address_space_->pgtable().map(ptalloc_, 0xffff'ffff'c000'0000, GB(1), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);

	// Activate the mapping (flushing the TLB along the way)
	root_address_space_->pgtable().activate();
}

bool memory_manager::try_handle_page_fault(u64 faulting_address) { return false; }
