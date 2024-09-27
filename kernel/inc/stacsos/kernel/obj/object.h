/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/sched/process.h>
#include <stacsos/kernel/sched/thread.h>
#include <stacsos/memory.h>

namespace stacsos::kernel::obj {
enum class operation_result_code : u64 { ok = 1, not_found = 2, not_supported = 3 };

struct operation_result {
	operation_result_code code;
	u64 data;

	static operation_result ok(u64 data = 0) { return operation_result { operation_result_code::ok, data }; }
	static operation_result not_supported() { return operation_result { operation_result_code::not_supported, 0 }; }
};

class object {
public:
	virtual ~object() { }

	u64 id() const { return id_; }

	virtual operation_result read(void *buffer, size_t length) { return operation_result::not_supported(); }
	virtual operation_result pread(void *buffer, size_t length, size_t offset) { return operation_result::not_supported(); }
	virtual operation_result write(const void *buffer, size_t length) { return operation_result::not_supported(); }
	virtual operation_result pwrite(const void *buffer, size_t length, size_t offset) { return operation_result::not_supported(); }
	virtual operation_result ioctl(u64 cmd, void *buffer, size_t length) { return operation_result::not_supported(); }
	virtual operation_result wait_for_status_change() { return operation_result::not_supported(); }
	virtual operation_result join() { return operation_result::not_supported(); }

protected:
	object(u64 id)
		: id_(id)
	{
	}

private:
	u64 id_;
};

class file_object : public object {
public:
	file_object(u64 id, shared_ptr<fs::file> file)
		: object(id)
		, file_(file)
	{
	}

	virtual operation_result read(void *buffer, size_t length) { return operation_result::ok(file_->read(buffer, length)); }
	virtual operation_result pread(void *buffer, size_t length, size_t offset) { return operation_result::ok(file_->pread(buffer, offset, length)); }
	virtual operation_result write(const void *buffer, size_t length) { return operation_result::ok(file_->write(buffer, length)); }
	virtual operation_result pwrite(const void *buffer, size_t length, size_t offset) { return operation_result::ok(file_->pwrite(buffer, offset, length)); }
	virtual operation_result ioctl(u64 cmd, void *buffer, size_t length) { return operation_result::ok(file_->ioctl(cmd, buffer, length)); }

private:
	shared_ptr<fs::file> file_;
};

class process_object : public object {
public:
	process_object(u64 id, shared_ptr<sched::process> proc)
		: object(id)
		, proc_(proc)
	{
	}

	virtual operation_result wait_for_status_change() override
	{
		sched::process_state initial = proc_->state();

		while (proc_->state() == initial) {
			proc_->state_changed_event().wait();
		}

		return operation_result::ok(0);
	}

private:
	shared_ptr<sched::process> proc_;
};

class thread_object : public object {
public:
	thread_object(u64 id, shared_ptr<sched::thread> thread)
		: object(id)
		, thread_(thread)
	{
	}

	virtual operation_result join() override
	{
		while (thread_->state() != sched::thread_states::terminated) {
			thread_->state_changed_event().wait();
		}

		return operation_result::ok(0);
	}

private:
	shared_ptr<sched::thread> thread_;
};
} // namespace stacsos::kernel::obj
