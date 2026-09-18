#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/page-table-allocator.h>

using namespace stacsos::kernel::mem;

page &page_table_allocator::allocate() { return memory_manager::get().pgalloc().allocate_pages(0, page_allocation_flags::zero).to_page(); }

void page_table_allocator::free(page &pg) { memory_manager::get().pgalloc().free_pages(pg.pfn(), 0); }
