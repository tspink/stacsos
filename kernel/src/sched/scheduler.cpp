/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#include <stacsos/kernel/arch/core-manager.h>
#include <stacsos/kernel/arch/core.h>
#include <stacsos/kernel/sched/schedulable-entity.h>
#include <stacsos/kernel/sched/scheduler.h>

using namespace stacsos::kernel::sched;
using namespace stacsos::kernel::arch;

// TODO: support multicore.

void scheduler::add_to_schedule(schedulable_entity &e) { core_manager::get().get_core(0).add_to_runqueue(*e.get_tcb()); }

void scheduler::remove_from_schedule(schedulable_entity &e) { core_manager::get().get_core(0).remove_from_runqueue(*e.get_tcb()); }
