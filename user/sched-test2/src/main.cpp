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

static void *thread_proc(void *arg)
{
	unsigned int thread_num = (unsigned int)(unsigned long)arg;

	console::get().writef("Thread %d running!\n", thread_num);

	syscalls::sleep(1500 + (thread_num * 500));

	console::get().writef("Thread %d stopping!\n", thread_num);

	return nullptr;
}

int main(const char *cmdline)
{
	console::get().write("Running Scheduler Test 2...\n");

	thread *threads[10];
	console::get().writef("Using %d threads...\n", ARRAY_SIZE(threads));
	for (unsigned int i = 0; i < ARRAY_SIZE(threads); i++) {
		threads[i] = thread::start(thread_proc, (void *)(unsigned long)i);
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(threads); i++) {
		threads[i]->join();
	}

	console::get().write("Scheduler test complete.\n");
	return 0;
}
