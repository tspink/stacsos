/* SPDX-License-Identifier: MIT */

/* StACSOS - scheduler test utility
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/console.h>
#include <stacsos/threads.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

static void *thread1(void *arg)
{
	while (true) {
		console::get().write("thread 1\n");
		syscalls::sleep(1000);
	}

	return nullptr;
}

static void *thread2(void *arg)
{
	while (true) {
		console::get().write("thread 2\n");
		syscalls::sleep(1500);
	}

	return nullptr;
}

int main(const char *cmdline)
{
	thread *ta = thread::start(thread1);
	thread *tb = thread::start(thread2);

	ta->join();
	tb->join();

	return 0;
}
