/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/bitset.h>

namespace stacsos::kernel::mem {
enum class slab_state { empty, partial, full };

template <size_t object_size, int slab_page_order> class slab_cache {
private:
	static const size_t slab_memory_size = ((1u << slab_page_order) * PAGE_SIZE);
	static const size_t slab_object_capacity = slab_memory_size / object_size;

	class slab {
		friend class slab_cache;

		static const size_t header_size = sizeof(slab);
		static const size_t reserved_objects = (header_size + (object_size - 1)) / object_size;

		friend class cache;

		using object_index_type = u64;

	public:
		slab()
			: next_(nullptr)
			, used_count_(0)
		{
			for (u64 i = 0; i < reserved_objects; i++) {
				used_[i] = true;
				used_count_++;
			}
		}

		slab_state state() const
		{
			return (used_objects() == 0) ? slab_state::empty : ((used_objects() == capacity()) ? slab_state::full : slab_state::partial);
		}

		size_t capacity() const { return slab_object_capacity; }

		size_t used_objects() const { return used_count_; }

		void *allocate()
		{
			assert(state() != slab_state::full);

			object_index_type free_object = used_.find_first_zero();

			if (free_object == ~0ull) {
				panic("slab is full!");
			}

			used_[free_object] = true;
			used_count_++;

			return object_ptr(free_object);
		}

		void free(void *ptr)
		{
			u64 used_object = index_of(ptr);

			used_[used_object] = false;
			used_count_--;
		}

		bool contains_object(void *ptr) { return ((uintptr_t)ptr >= (uintptr_t)this) && ((uintptr_t)ptr < (uintptr_t)this + slab_memory_size); }

		void *object_ptr(object_index_type object_index) { return (void *)((uintptr_t)this + (object_index * object_size)); }

		object_index_type index_of(void *object_ptr) { return ((uintptr_t)object_ptr - (uintptr_t)this) / object_size; }

	private:
		slab *next_;
		size_t used_count_;
		stacsos::bitset<slab_object_capacity> used_;
	};

public:
	slab_cache()
		: slabs_(nullptr)
	{
	}

	void *allocate()
	{
		// Find a free slab
		slab *s = slabs_;
		while (s && s->state() == slab_state::full) {
			s = s->next_;
		}

		if (!s) {
			// Allocate a new slab
			void *slab_base = allocate_slab();
			if (!slab_base) {
				panic("out of memory");
			}

			s = new (slab_base) slab();
			s->next_ = slabs_;
			slabs_ = s;
		}

		void *ptr = s->allocate();
		// dprintf("malloc: cache-size=%u, slab=%p, ptr=%p\n", object_size, s, ptr);
		return ptr;
	}

	bool try_free(void *ptr)
	{
		// Find containing slab
		slab *s = slabs_;
		while (s) {
			if (s->contains_object(ptr)) {
				break;
			}

			s = s->next_;
		}

		if (!s) {
			return false;
		}

		s->free(ptr);
		// dprintf("free: ptr=%p\n", ptr);

		// TODO: Free slabs?

		return true;
	}

	void free(void *ptr)
	{
		if (!try_free(ptr)) {
			panic("object not in cache\n");
		}
	}

private:
	slab *slabs_;

	void *allocate_slab();
};
} // namespace stacsos::kernel::mem
