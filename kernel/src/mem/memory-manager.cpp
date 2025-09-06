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

struct exclusion {
	u64 start, length;
};

void memory_manager::initialise_page_allocator(u64 nr_page_descriptors)
{
	// Determine whether or not we're running in self-test mode for the page allocator.
	if (memops::strcmp(config::get().get_option_or_default("pgalloc-selftest", "no"), "yes") == 0) {
		// Do the self-test, which should hang the system.
		pgalloc_->perform_selftest();

		// Which means, we never get here.
		__unreachable();
	}

	dprintf("mem: initialising page allocator...\n");

	// Define the memory exclusion ranges, so that we don't add these to the page allocator's free lists.
	// NOTE: This list *MUST* be ordered on base address.
	static const exclusion exclusions[] = {
		{ 0, MB(1) }, // Early BIOS data, and the ZERO page.
		{ 0x100000, KB(24) }, // 24 kB (6 pages) of early page tables -- we should probably put these back later.
		{ (u64)&_IMAGE_START, PAGE_ALIGN_UP((u64)&_IMAGE_END) - ((u64)&_IMAGE_START) }, // The loaded kernel image,
		{ (u64)&_DYNAMIC_DATA_START - 0xffff'ffff'8000'0000,
			PAGE_ALIGN_UP(sizeof(page) * nr_page_descriptors) } // Dynamic data, containing the page descriptors.
	};

	dprintf("excluion range:\n");
	for (int i = 0; i < ARRAY_SIZE(exclusions); i++) {
		dprintf("  %016lx -- %016lx\n", exclusions[i].start, exclusions[i].start + exclusions[i].length);
	}

	dprintf("registering free memory blocks...\n");
	// Loop through each memory block that we received from the
	// startup code.
	for (int i = 0; i < nr_memory_blocks; i++) {
		const memory_block *mb = &memory_blocks[i];

		// If the memory block is available, then insert them into the page allocator.
		if (mb->avail) {
			auto free_range_base = mb->start;
			auto free_range_length = (mb->length & ~PAGE_BITS); // Make sure we align down
			auto free_range_end = free_range_base + free_range_length;

			dprintf("candidate memory block: %016lx -- %016lx\n", free_range_base, free_range_end);

			auto max_end = free_range_end;
			while (free_range_base < free_range_end) {
				dprintf("  considering %016lx -- %016lx\n", free_range_base, free_range_end);

				// Find any exclusions that this candidate free range intersects
				bool retry = false;
				for (int i = 0; i < ARRAY_SIZE(exclusions); i++) {
					if (free_range_base >= exclusions[i].start && free_range_base < (exclusions[i].start + exclusions[i].length)) {
						dprintf("  range start intersects exclusion %016lx -- %016lx\n", exclusions[i].start, exclusions[i].start + exclusions[i].length);
						free_range_base = exclusions[i].start + exclusions[i].length;
						retry = true;
						break;
					}
				}

				if (retry) {
					continue;
				}

				// At this point, we have a free range base that doesn't intersect with any exclusion.
				// But, we need to make sure there are no exclusions within this candidate range.
				max_end = free_range_end;
				for (int i = 0; i < ARRAY_SIZE(exclusions); i++) {
					if (exclusions[i].start >= free_range_base && exclusions[i].start < max_end) {
						// We've found an exclusion that is within this free range
						dprintf(
							"  exclusion intersects candidate free range %016lx -- %016lx\n", exclusions[i].start, exclusions[i].start + exclusions[i].length);
						max_end = exclusions[i].start;
						break;
					}
				}

				dprintf("  free range chunk %016lx -- %016lx\n", free_range_base, max_end);

				// Add these pages to the page allocator
				pgalloc_->insert_free_pages(page::get_from_base_address(free_range_base), (max_end - free_range_base) >> PAGE_BITS);

				free_range_base = max_end;
			}
		} else {
			// Otherwise, mark these pages as reserved.
			for (u64 pfn = (mb->start >> PAGE_BITS); pfn < ((mb->start + mb->length) >> PAGE_BITS); pfn++) {
				auto &pg = page::get_from_pfn(pfn);
				pg.type_ = page_type::reserved;
			}
		}
	}
}

void memory_manager::initialise_object_allocator()
{
	// Nothing to do to initialise the object allocator!
}

void memory_manager::activate_primary_mapping()
{
	root_address_space_ = new address_space(ptalloc_, (u64)0);

	// Insert a mapping that allows us to access physical memory 1-1 (4 gigabytyes) -- this allows the phys_to_virt()
	// function to work, and is highly convenient.
	u64 phys_base = 0;

	// TODO: Should do this up until the last physical memory block.
	for (int i = 0; i < 12; i++) {
		root_address_space_->pgtable().map(
			ptalloc_, 0xffff'8000'0000'0000 + phys_base, phys_base, mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
		phys_base += GB(1);
	}

	// This mapping is for the kernel high address space.  It's used mainly for executing kernel code, and is how gcc compiles
	// the kernel code with -mcmodel=kernel
	root_address_space_->pgtable().map(ptalloc_, 0xffff'ffff'8000'0000, GB(0), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);
	root_address_space_->pgtable().map(ptalloc_, 0xffff'ffff'c000'0000, GB(1), mapping_flags::present | mapping_flags::writable, mapping_size::m1g);

	// Activate the mapping (flushing the TLB along the way)
	root_address_space_->pgtable().activate();
}

bool memory_manager::try_handle_page_fault(u64 faulting_address) { return false; }
