/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/syscalls.h>

namespace stacsos {
struct rw_result {
	syscall_result_code code;
	u64 length;
};

struct fa_result {
	syscall_result_code code;
	u64 id;
};

struct alloc_result {
	syscall_result_code code;
	void *ptr;
};

class syscalls {
public:
	static syscall_result_code exit(u64 result) { return syscall1(syscall_numbers::exit, result).code; }

	static syscall_result_code set_fs(u64 value) { return syscall1(syscall_numbers::set_fs, value).code; }
	static syscall_result_code set_gs(u64 value) { return syscall1(syscall_numbers::set_gs, value).code; }

	static fa_result open(const char *path)
	{
		auto r = syscall1(syscall_numbers::open, (u64)path);
		return fa_result { r.code, r.data };
	}

	static syscall_result_code close(u64 id) { return syscall1(syscall_numbers::close, id).code; }

	static rw_result read(u64 object, void *buffer, u64 length)
	{
		auto r = syscall3(syscall_numbers::read, object, (u64)buffer, length);
		return rw_result { r.code, r.data };
	}

	static rw_result write(u64 object, const void *buffer, u64 length)
	{
		auto r = syscall3(syscall_numbers::write, object, (u64)buffer, length);
		return rw_result { r.code, r.data };
	}

	static rw_result pwrite(u64 object, const void *buffer, u64 length, size_t offset)
	{
		auto r = syscall4(syscall_numbers::pwrite, object, (u64)buffer, length, offset);
		return rw_result { r.code, r.data };
	}

	static rw_result pread(u64 object, void *buffer, u64 length, size_t offset)
	{
		auto r = syscall4(syscall_numbers::pread, object, (u64)buffer, length, offset);
		return rw_result { r.code, r.data };
	}

	static rw_result ioctl(u64 object, u64 cmd, void *buffer, u64 length)
	{
		auto r = syscall4(syscall_numbers::ioctl, object, cmd, (u64)buffer, length);
		return rw_result { r.code, r.data };
	}

	static alloc_result alloc_mem(u64 size)
	{
		auto r = syscall1(syscall_numbers::alloc_mem, size);
		return alloc_result { r.code, (void *)r.data };
	}

	static syscall_result start_process(const char *path, const char *args) { return syscall2(syscall_numbers::start_process, (u64)path, (u64)args); }
	static syscall_result wait_process(u64 id) { return syscall1(syscall_numbers::wait_for_process, id); }

	static syscall_result start_thread(void *entrypoint, void *arg) { return syscall2(syscall_numbers::start_thread, (u64)entrypoint, (u64)arg); }
	static syscall_result join_thread(u64 id) { return syscall1(syscall_numbers::join_thread, id); }
	static syscall_result stop_current_thread() { return syscall0(syscall_numbers::stop_current_thread); }

	static syscall_result sleep(u64 ms) { return syscall1(syscall_numbers::sleep, ms); }

	static void poweroff() { syscall0(syscall_numbers::poweroff); }

private:
	static syscall_result syscall0(syscall_numbers id)
	{
		syscall_result r;
		asm volatile("syscall" : "=a"(r.code), "=d"(r.data) : "a"(id) : "flags", "rcx");
		return r;
	}

	static syscall_result syscall1(syscall_numbers id, u64 arg0)
	{
		syscall_result r;
		asm volatile("syscall" : "=a"(r.code), "=d"(r.data) : "a"(id), "D"(arg0) : "flags", "rcx");
		return r;
	}

	static syscall_result syscall2(syscall_numbers id, u64 arg0, u64 arg1)
	{
		syscall_result r;
		asm volatile("syscall" : "=a"(r.code), "=d"(r.data) : "a"(id), "D"(arg0), "S"(arg1) : "flags", "rcx");
		return r;
	}

	static syscall_result syscall3(syscall_numbers id, u64 arg0, u64 arg1, u64 arg2)
	{
		syscall_result r;
		asm volatile("syscall" : "=a"(r.code), "=d"(r.data) : "a"(id), "D"(arg0), "S"(arg1), "d"(arg2) : "flags", "rcx");
		return r;
	}

	static syscall_result syscall4(syscall_numbers id, u64 arg0, u64 arg1, u64 arg2, u64 arg3)
	{
		u64 forced_arg3 asm("r8") = arg3;

		syscall_result r;
		asm volatile("syscall" : "=a"(r.code), "=d"(r.data) : "a"(id), "D"(arg0), "S"(arg1), "d"(arg2), "r"(forced_arg3) : "flags", "rcx", "r11");
		return r;
	}
};
} // namespace stacsos
