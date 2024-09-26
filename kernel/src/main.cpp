/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/x86/x86-platform.h>
#include <stacsos/kernel/config.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/console/physical-console.h>
#include <stacsos/kernel/dev/console/virtual-console.h>
#include <stacsos/kernel/dev/devfs.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/gfx/qemu-stdvga.h>
#include <stacsos/kernel/dev/input/keyboard.h>
#include <stacsos/kernel/dev/storage/ahci-storage-device.h>
#include <stacsos/kernel/dev/tty/terminal.h>
#include <stacsos/kernel/dev/misc/cmos-rtc.h>
#include <stacsos/kernel/fs/filesystem.h>
#include <stacsos/kernel/fs/vfs.h>
#include <stacsos/kernel/mem/memory-manager.h>
#include <stacsos/kernel/sched/process-manager.h>
#include <stacsos/memops.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::fs;
using namespace stacsos::kernel::dev;
using namespace stacsos::kernel::dev::storage;
using namespace stacsos::kernel::dev::gfx;
using namespace stacsos::kernel::dev::console;
using namespace stacsos::kernel::dev::tty;
using namespace stacsos::kernel::dev::input;
using namespace stacsos::kernel::dev::misc;
using namespace stacsos::kernel::sched;

static void init_console()
{
	auto &dm = device_manager::get();

	auto rtc = new cmos_rtc(dm.sysbus());
	dm.register_device(*rtc);

	auto kbd = new keyboard(dm.sysbus());
	dm.register_device(*kbd);

	auto phys_console = new physical_console(dm.sysbus(), dm.get_device_by_class<qemu_stdvga>(qemu_stdvga::qemu_stdvga_device_class));

	dm.register_device(*phys_console);

	const char *console_mode_option = config::get().get_option("console-mode");
	virtual_console_mode console_mode = virtual_console_mode::text;
	if (console_mode_option && stacsos::memops::strcmp(config::get().get_option("console-mode"), "gfx") == 0) {
		console_mode = virtual_console_mode::gfx;
	}

	auto vc0 = new virtual_console(dm.sysbus(), console_mode);
	dm.register_device(*vc0);

	auto tty0 = new terminal(dm.sysbus());
	tty0->attach(*vc0);
	dm.register_device(*tty0);
	dm.add_device_alias(*tty0, "console");

	auto vc1 = new virtual_console(dm.sysbus(), console_mode);
	dm.register_device(*vc1);

	auto tty1 = new terminal(dm.sysbus());
	tty1->attach(*vc1);
	dm.register_device(*tty1);

	tty1->write_line("This is virtual console #2.  The debug logs are here!");

	dprintf_set_console(tty1);
	dprintf("dbg: switched to tty1\n");

	phys_console->add_virtual_console(*vc0);
	phys_console->add_virtual_console(*vc1);

	// abort();
}

static void continue_main()
{
	device_manager::get().probe_buses();
	init_console();

	// Mount the root filesystem
	auto *root = vfs::get().lookup("/");
	if (!root) {
		panic("unable to acquire fs root");
	}

	auto &dev = device_manager::get().get_device_by_class<ahci_storage_device>(ahci_storage_device::ahci_storage_device_class);
	auto *fs = filesystem::create_from_bdev(dev, fs_type_hint::tarfs);
	if (!fs) {
		panic("unable to create filesystem");
	}

	root->mount(*fs);

	auto *devfs_dir = vfs::get().lookup("/")->mkdir("dev");
	if (!devfs_dir) {
		panic("unable to create directory for devfs");
	}

	devfs_dir->mount(*new devfs());

	// Launch the init process
	auto init_proc = process_manager::get().create_process("/usr/init", "");
	if (!init_proc) {
		panic("unable to create init process");
	}

	init_proc->start();
}

__noreturn void main(const char *cmdline)
{
	debug_helper::get().parse_image();
	stacsos::kernel::config::get().init(cmdline);

	// Initialise the memory manager first, so we can allocate memory.
	stacsos::kernel::mem::memory_manager::get().init();

	// Now, initialise the core manager, which looks after CPU resources.
	stacsos::kernel::arch::core_manager::get().init();

	// Next, initialise the VFS.
	stacsos::kernel::fs::vfs::get().init();

	// Initialise the device manager, and probe the platform.
	stacsos::kernel::dev::device_manager::get().init();
	stacsos::kernel::arch::x86::x86_platform::get().probe();

	// Initialise the process manager, so we can start running threads.
	stacsos::kernel::sched::process_manager::get().init();

	// Create the kernel process, and start it.
	auto kp = stacsos::kernel::sched::process_manager::get().create_kernel_process(continue_main);
	kp->start();

	// Tell the core manager to begin executing processes.
	stacsos::kernel::arch::core_manager::get().go();
}
