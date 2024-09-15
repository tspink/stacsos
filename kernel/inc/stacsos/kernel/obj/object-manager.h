/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/atomic.h>
#include <stacsos/kernel/obj/object.h>
#include <stacsos/map.h>

namespace stacsos::kernel::sched {
class process;
} // namespace stacsos::kernel::sched

namespace stacsos::kernel::obj {
class object_manager {
	DEFINE_SINGLETON(object_manager);

private:
	object_manager()
		: next_id_(1)
	{
	}

public:
	shared_ptr<object> get_object(sched::process &owner, u64 id)
	{
		map<u64, shared_ptr<object>> *process_object_map;
		if (!objects_.try_get_value(&owner, process_object_map)) {
			return nullptr;
		}

		shared_ptr<object> optr;
		if (!process_object_map->try_get_value(id, optr)) {
			return nullptr;
		}

		return optr;
	}

	void free_object(sched::process &owner, u64 id) { }

	shared_ptr<object> create_file_object(sched::process &owner, shared_ptr<fs::file> file)
	{
		return register_object(owner, new file_object(allocate_id(owner), file));
	}

	shared_ptr<object> create_process_object(sched::process &owner, shared_ptr<sched::process> proc)
	{
		return register_object(owner, new process_object(allocate_id(owner), proc));
	}

	shared_ptr<object> create_thread_object(sched::process &owner, shared_ptr<sched::thread> thread)
	{
		return register_object(owner, new thread_object(allocate_id(owner), thread));
	}

private:
	atomic_u64 next_id_;
	map<sched::process *, map<u64, shared_ptr<object>> *> objects_;

	u64 allocate_id(sched::process &owner)
	{
		u64 n = next_id_++;

		dprintf("OM: %lu\n", n);

		return n;
	}

	shared_ptr<object> register_object(sched::process &owner, object *o)
	{
		map<u64, shared_ptr<object>> *process_object_map;
		if (!objects_.try_get_value(&owner, process_object_map)) {
			process_object_map = new map<u64, shared_ptr<object>>();
			objects_.add(&owner, process_object_map);
		}

		auto object_ptr = shared_ptr(o);

		process_object_map->add(o->id(), object_ptr);
		return object_ptr;
	}
};
} // namespace stacsos::kernel::obj
