/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/x86/machine-context.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos::kernel::arch::x86;

static const char *FLAG_CODES[]
	= { "CF", "(1)", "PF", "", "AF", "", "ZF", "SF", "TF", "IF", "DF", "OF", "P0", "P1", "NT", "MD", "RF", "VM", "AC", "VIF", "VIP", "ID" };

void machine_context::dump() const
{
	dprintf("MACHINE CONTEXT %p\n", this);
	dprintf("RAX=%016lx    RCX=%016lx\n", rax, rcx);
	dprintf("RDX=%016lx    RBX=%016lx\n", rdx, rbx);
	dprintf("RDI=%016lx    RSI=%016lx\n", rdi, rsi);
	dprintf("RBP=%016lx    RSP=%016lx\n", rbp, rsp);
	dprintf("R8 =%016lx    R9 =%016lx\n", r8, r9);
	dprintf("R10=%016lx    R11=%016lx\n", r10, r11);
	dprintf("R12=%016lx    R13=%016lx\n", r12, r13);
	dprintf("R14=%016lx    R15=%016lx\n", r14, r15);
	dprintf("FS =%016lx    GS =%016lx\n", fs, gs);
	dprintf("RFLAGS=%08lx ", rflags);

	for (unsigned int i = 0; i < sizeof(FLAG_CODES) / sizeof(FLAG_CODES[0]); i++) {
		if (rflags & (1 << i)) {
			dprintf("%s ", FLAG_CODES[i]);
		}
	}

	dprintf("\n");

	dprintf("CS=%04lx  SS=%04lx\n", cs, ss);
	dprintf("RIP=%016lx\n", rip);
}
