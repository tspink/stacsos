/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/mem/object-allocator.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::mem;

extern "C" {
void *__dso_handle = &__dso_handle;
int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso) { return 0; }
}

void *operator new(size_t size) { return memory_manager::get().objalloc().alloc(size); }

void *operator new[](size_t size) { return memory_manager::get().objalloc().alloc(size); }

void operator delete(void *p) { memory_manager::get().objalloc().free(p); }

void operator delete[](void *p) { memory_manager::get().objalloc().free(p); }

void operator delete[](void *p, size_t sz) { memory_manager::get().objalloc().free(p); }

void operator delete(void *p, size_t sz) { memory_manager::get().objalloc().free(p); }
