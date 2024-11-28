/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/user-syscall.h>

extern "C" {
void *__dso_handle = &__dso_handle;
int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso) { return 0; }
}

struct memory_block;
static memory_block *free_list;
struct memory_block {
	memory_block *next, *prev;
	size_t size;

	void remove()
	{
		memory_block **slot = &free_list;
		while (*slot) {
			if (*slot == this) {
				*slot = this->next;
				return;
			}
		}
	}

	void insert()
	{
		memory_block **slot = &free_list;
		while (*slot) {
			slot = &(*slot)->next;
		}

		*slot = this;
	}

	memory_block *split(size_t size)
	{
		size_t orig_size = this->size;
		this->size = size;

		memory_block *new_block = (memory_block *)((u64)this + size + sizeof(memory_block));
		new_block->next = nullptr;
		new_block->prev = nullptr;
		new_block->size = orig_size - size;

		return new_block;
	}

	void *ptr() { return (void *)((u64)this + sizeof(this)); }
};

static void *allocate(size_t size)
{
	memory_block *candidate_block = free_list;

	while (candidate_block) {
		if (candidate_block->size >= size) {
			candidate_block->remove();

			if (candidate_block->size > (size - sizeof(memory_block))) {
				memory_block *new_block = candidate_block->split(size);
				new_block->insert();
			}

			return candidate_block->ptr();
		}

		candidate_block = candidate_block->next;
	}

	size_t new_size = max(0x1000ul, (size + sizeof(memory_block) + 0xfff) & ~0xfff);
	auto alloc_result = stacsos::syscalls::alloc_mem(new_size);
	if (alloc_result.code != stacsos::syscall_result_code::ok) {
		return nullptr;
	}

	candidate_block = (memory_block *)alloc_result.ptr;
	candidate_block->next = nullptr;
	candidate_block->prev = nullptr;
	candidate_block->size = new_size - sizeof(memory_block);

	if (new_size > size) {
		memory_block *new_block = candidate_block->split(size);
		new_block->insert();
	}

	return candidate_block->ptr();
}

void free(void *ptr)
{
	//
}

void *operator new(size_t size) { return allocate(size); }

void *operator new[](size_t size) { return operator new(size); }

void operator delete(void *p) { free(p); }

void operator delete[](void *p) { operator delete(p); }

void operator delete[](void *p, size_t sz) { operator delete(p); }

void operator delete(void *p, size_t sz) { operator delete(p); }
