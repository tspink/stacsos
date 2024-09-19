#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-table-allocator.h>

using namespace stacsos::kernel::mem;

page *page_table_allocator::allocate()
{
	page *p = memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero);
	if (p == nullptr) {
		panic("unable to allocate page table");
	}

	return p;
}

void page_table_allocator::free(page *pg) { memory_manager::get().pgalloc().free_pages(*pg, 0); }
