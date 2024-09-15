/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/arch/x86/irq/irq-manager.h>
#include <stacsos/kernel/arch/x86/irq/irq-traps.h>
#include <stacsos/kernel/arch/x86/x86-core.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/process-manager.h>
#include <stacsos/kernel/sched/thread.h>

using namespace stacsos::kernel;
using namespace stacsos::kernel::arch::x86;
using namespace stacsos::kernel::arch::x86::irq;

static irq_trap_fn_type irq_trap_functions[] = { x86_irq_trap_0, x86_irq_trap_1, x86_irq_trap_2, x86_irq_trap_3, x86_irq_trap_4, x86_irq_trap_5, x86_irq_trap_6,
	x86_irq_trap_7, x86_irq_trap_8, x86_irq_trap_9, x86_irq_trap_10, x86_irq_trap_11, x86_irq_trap_12, x86_irq_trap_13, x86_irq_trap_14, x86_irq_trap_15,
	x86_irq_trap_16, x86_irq_trap_17, x86_irq_trap_18, x86_irq_trap_19, x86_irq_trap_20, x86_irq_trap_21, x86_irq_trap_22, x86_irq_trap_23, x86_irq_trap_24,
	x86_irq_trap_25, x86_irq_trap_26, x86_irq_trap_27, x86_irq_trap_28, x86_irq_trap_29, x86_irq_trap_30, x86_irq_trap_31, x86_irq_trap_32, x86_irq_trap_33,
	x86_irq_trap_34, x86_irq_trap_35, x86_irq_trap_36, x86_irq_trap_37, x86_irq_trap_38, x86_irq_trap_39, x86_irq_trap_40, x86_irq_trap_41, x86_irq_trap_42,
	x86_irq_trap_43, x86_irq_trap_44, x86_irq_trap_45, x86_irq_trap_46, x86_irq_trap_47, x86_irq_trap_48, x86_irq_trap_49, x86_irq_trap_50, x86_irq_trap_51,
	x86_irq_trap_52, x86_irq_trap_53, x86_irq_trap_54, x86_irq_trap_55, x86_irq_trap_56, x86_irq_trap_57, x86_irq_trap_58, x86_irq_trap_59, x86_irq_trap_60,
	x86_irq_trap_61, x86_irq_trap_62, x86_irq_trap_63, x86_irq_trap_64, x86_irq_trap_65, x86_irq_trap_66, x86_irq_trap_67, x86_irq_trap_68, x86_irq_trap_69,
	x86_irq_trap_70, x86_irq_trap_71, x86_irq_trap_72, x86_irq_trap_73, x86_irq_trap_74, x86_irq_trap_75, x86_irq_trap_76, x86_irq_trap_77, x86_irq_trap_78,
	x86_irq_trap_79, x86_irq_trap_80, x86_irq_trap_81, x86_irq_trap_82, x86_irq_trap_83, x86_irq_trap_84, x86_irq_trap_85, x86_irq_trap_86, x86_irq_trap_87,
	x86_irq_trap_88, x86_irq_trap_89, x86_irq_trap_90, x86_irq_trap_91, x86_irq_trap_92, x86_irq_trap_93, x86_irq_trap_94, x86_irq_trap_95, x86_irq_trap_96,
	x86_irq_trap_97, x86_irq_trap_98, x86_irq_trap_99, x86_irq_trap_100, x86_irq_trap_101, x86_irq_trap_102, x86_irq_trap_103, x86_irq_trap_104,
	x86_irq_trap_105, x86_irq_trap_106, x86_irq_trap_107, x86_irq_trap_108, x86_irq_trap_109, x86_irq_trap_110, x86_irq_trap_111, x86_irq_trap_112,
	x86_irq_trap_113, x86_irq_trap_114, x86_irq_trap_115, x86_irq_trap_116, x86_irq_trap_117, x86_irq_trap_118, x86_irq_trap_119, x86_irq_trap_120,
	x86_irq_trap_121, x86_irq_trap_122, x86_irq_trap_123, x86_irq_trap_124, x86_irq_trap_125, x86_irq_trap_126, x86_irq_trap_127, x86_irq_trap_128,
	x86_irq_trap_129, x86_irq_trap_130, x86_irq_trap_131, x86_irq_trap_132, x86_irq_trap_133, x86_irq_trap_134, x86_irq_trap_135, x86_irq_trap_136,
	x86_irq_trap_137, x86_irq_trap_138, x86_irq_trap_139, x86_irq_trap_140, x86_irq_trap_141, x86_irq_trap_142, x86_irq_trap_143, x86_irq_trap_144,
	x86_irq_trap_145, x86_irq_trap_146, x86_irq_trap_147, x86_irq_trap_148, x86_irq_trap_149, x86_irq_trap_150, x86_irq_trap_151, x86_irq_trap_152,
	x86_irq_trap_153, x86_irq_trap_154, x86_irq_trap_155, x86_irq_trap_156, x86_irq_trap_157, x86_irq_trap_158, x86_irq_trap_159, x86_irq_trap_160,
	x86_irq_trap_161, x86_irq_trap_162, x86_irq_trap_163, x86_irq_trap_164, x86_irq_trap_165, x86_irq_trap_166, x86_irq_trap_167, x86_irq_trap_168,
	x86_irq_trap_169, x86_irq_trap_170, x86_irq_trap_171, x86_irq_trap_172, x86_irq_trap_173, x86_irq_trap_174, x86_irq_trap_175, x86_irq_trap_176,
	x86_irq_trap_177, x86_irq_trap_178, x86_irq_trap_179, x86_irq_trap_180, x86_irq_trap_181, x86_irq_trap_182, x86_irq_trap_183, x86_irq_trap_184,
	x86_irq_trap_185, x86_irq_trap_186, x86_irq_trap_187, x86_irq_trap_188, x86_irq_trap_189, x86_irq_trap_190, x86_irq_trap_191, x86_irq_trap_192,
	x86_irq_trap_193, x86_irq_trap_194, x86_irq_trap_195, x86_irq_trap_196, x86_irq_trap_197, x86_irq_trap_198, x86_irq_trap_199, x86_irq_trap_200,
	x86_irq_trap_201, x86_irq_trap_202, x86_irq_trap_203, x86_irq_trap_204, x86_irq_trap_205, x86_irq_trap_206, x86_irq_trap_207, x86_irq_trap_208,
	x86_irq_trap_209, x86_irq_trap_210, x86_irq_trap_211, x86_irq_trap_212, x86_irq_trap_213, x86_irq_trap_214, x86_irq_trap_215, x86_irq_trap_216,
	x86_irq_trap_217, x86_irq_trap_218, x86_irq_trap_219, x86_irq_trap_220, x86_irq_trap_221, x86_irq_trap_222, x86_irq_trap_223, x86_irq_trap_224,
	x86_irq_trap_225, x86_irq_trap_226, x86_irq_trap_227, x86_irq_trap_228, x86_irq_trap_229, x86_irq_trap_230, x86_irq_trap_231, x86_irq_trap_232,
	x86_irq_trap_233, x86_irq_trap_234, x86_irq_trap_235, x86_irq_trap_236, x86_irq_trap_237, x86_irq_trap_238, x86_irq_trap_239, x86_irq_trap_240,
	x86_irq_trap_241, x86_irq_trap_242, x86_irq_trap_243, x86_irq_trap_244, x86_irq_trap_245, x86_irq_trap_246, x86_irq_trap_247, x86_irq_trap_248,
	x86_irq_trap_249, x86_irq_trap_250, x86_irq_trap_251, x86_irq_trap_252, x86_irq_trap_253, x86_irq_trap_254, x86_irq_trap_255 };

extern "C" void x86_handle_irq(u8 irq_number, void *mcontext) { x86_core::this_core().irqmgr().handle_irq(irq_number, mcontext); }

static void unhandled_interrupt(u8 irq_number, void *mcontext, void *arg)
{
	machine_context *mc = (machine_context *)mcontext;
	auto &thread = stacsos::kernel::sched::thread::current();
	dprintf("[%u] unhandled irq %u RIP=%p TASK=%p\n", stacsos::kernel::arch::core::this_core_id(), irq_number, mc->rip, &thread);
	mc->dump();

	u64 *ptr = (u64 *)mc->rbp;
	while (*ptr) {
		dprintf("Ret Addr=%p\n", ptr[1]);
		ptr = (u64 *)ptr[0];
	}

	abort();
}

template <int NR_IRQS> void irq_manager<NR_IRQS>::initialise()
{
	// Mark all the exception vectors as used.
	for (int i = 0; i < 32; i++) {
		used_irq_[i] = true;
	}

	// Fill in the IDT registrations with the corresponding trap function, and
	// populate the handler functions with the unhandled handler.
	for (int i = 0; i < idt_.NR_ENTRIES; i++) {
		idt_.register_interrupt_gate(i, (uintptr_t)irq_trap_functions[i], 8, descriptor_privilege_level::ring0);
		handlers_[i] = unhandled_interrupt;
		handler_args_[i] = nullptr;
	}

	// Reload the IDT
	idt_.reload();
}

template <int NR_IRQS> void irq_manager<NR_IRQS>::assign_irq(u8 irq_number, irq_handler_fn handler, void *arg)
{
	// Store the handler in the metadata structure.
	handlers_[irq_number] = handler;
	handler_args_[irq_number] = arg;
}

template <int NR_IRQS> void irq_manager<NR_IRQS>::reserve_irq(u8 irq_number, irq_handler_fn handler, void *arg)
{
	if (used_irq_[irq_number]) {
		panic("IRQ # already reserved");
	}

	used_irq_[irq_number] = true;
	assign_irq(irq_number, handler, arg);
}

template <int NR_IRQS> u8 irq_manager<NR_IRQS>::allocate_irq(irq_handler_fn handler, void *arg)
{
	// Find a free IRQ number, and reserve it.
	u64 irq = used_irq_.find_first_zero();
	reserve_irq(irq, handler, arg);

	dprintf("irq: allocated %d\n", irq);

	// Return the IRQ number that was just allocated.
	return (u8)irq;
}

template class irq_manager<256>;
