/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/elf.h>
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/fs/file.h>
#include <stacsos/kernel/fs/vfs.h>
#include <stacsos/kernel/mem/address-space.h>
#include <stacsos/kernel/sched/process-manager.h>
#include <stacsos/kernel/sched/thread.h>

using namespace stacsos;
using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::mem;
using namespace stacsos::kernel::arch;

void process_manager::init() { dprintf("processes: init\n"); }

shared_ptr<process> process_manager::create_kernel_process(continuation_fn cfn)
{
	auto kp = new process(exec_privilege::kernel);
	kp->create_thread((u64)cfn);

	auto kpp = shared_ptr(kp);
	active_processes_.append(kpp);

	return kpp;
}

shared_ptr<process> process_manager::create_process(const char *path, const char *args)
{
	auto *binary = stacsos::kernel::fs::vfs::get().lookup(path);
	if (!binary) {
		dprintf("pm: binary '%s' not found\n", path);
		return nullptr;
	}

	dprintf("pm: found binary\n");

	auto file = binary->open();
	if (!file) {
		dprintf("pm: unable to open binary\n");
		return nullptr;
	}

	char header_buffer[0x40];
	if (file->pread(header_buffer, 0, sizeof(header_buffer)) != sizeof(header_buffer)) {
		dprintf("pm: incorrect file size\n");
		return nullptr;
	}

	if (((const elf_ident_header *)header_buffer)->ei_class != elf_ident_classes::ei_class_64bit) {
		dprintf("pm: invalid elf class\n");
		return nullptr;
	}

	const elf_header<64> *ehdr = (const elf_header<64> *)header_buffer;

	// parse elf file
	// create process
	// load address space
	// create main thread

	auto proc = new process(exec_privilege::user);

	char *program_headers = new char[ehdr->e_phnum * ehdr->e_phentsize];
	file->pread(program_headers, ehdr->e_phoff, ehdr->e_phnum * ehdr->e_phentsize);

	for (int seg_idx = 0; seg_idx < ehdr->e_phnum; seg_idx++) {
		const elf_programheader<64> *phdr = ((const elf_programheader<64> *)(program_headers + (seg_idx * ehdr->e_phentsize)));

		// dprintf("hdr %d: type=%d flags=%x off=%lx vaddr=%lx paddr=%lx memsz=%lx filesz=%lx\n", seg_idx, phdr->p_type, phdr->p_flags, phdr->p_offset,
		// phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz, phdr->p_filesz);

		if (phdr->p_type == elf_program_header_type::pt_load) {
			u64 vaddr_page = phdr->p_vaddr & PAGE_MASK;
			u64 vaddr_page_offset = phdr->p_vaddr & ~PAGE_MASK;
			u64 size = (phdr->p_memsz + vaddr_page_offset + (PAGE_SIZE - 1)) & PAGE_MASK;

			auto rgn = proc->addrspace().add_region(vaddr_page, size, region_flags::all, true);
			if (!rgn) {
				panic("unable to add region for segment");
			}

			// rgn->storage
			void *target = (char *)rgn->storage->base_address_ptr() + vaddr_page_offset;
			// dprintf("copy to %p\n", target);
			file->pread(target, phdr->p_offset, phdr->p_filesz);
		}
	}

	delete[] program_headers;

	auto data_page = proc->addrspace().alloc_region(0x1000, region_flags::readable, true);
	if (!data_page) {
		panic("unable to allocate data page");
	}

	memops::strncpy((char *)data_page->storage->base_address_ptr(), args, memops::strlen(args) + 1);

	proc->create_thread(ehdr->e_entry, (void *)data_page->base);

	auto pp = shared_ptr(proc);
	active_processes_.append(pp);

	return pp;
}
