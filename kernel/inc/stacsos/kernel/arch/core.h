/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/kernel/arch/irq-manager.h>
#include <stacsos/kernel/arch/x86/msr.h>
#include <stacsos/kernel/config.h>
#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/sched/alg/rr.h>
#include <stacsos/kernel/sched/alg/scheduling-algorithm.h>
#include <stacsos/kernel/sched/alg/sfs.h>
#include <stacsos/kernel/sched/schedulable-entity.h>
#include <stacsos/list.h>

namespace stacsos::kernel::arch {
using namespace stacsos::kernel::sched;

class timer;
class core_manager;

enum class core_status { offline, online, error, bootstrap };

class core {
	friend class core_manager;

public:
	static int this_core_id() { return (int)x86::msrs::ia32_tsc_aux; }

	static core &this_core();

	explicit core(int id)
		: id_(id)
		, status_(core_status::offline)
		, irqs_(*this)
		, sched_alg_(nullptr)
	{
		idle_thread_.entity = nullptr;
		idle_thread_.mcontext = nullptr;

		//*new alg::simple_fair_scheduler()

		const char *sched_alg_name = config::get().get_option("sched");
		if (!sched_alg_name || *sched_alg_name == 0) {
			sched_alg_name = "sfs";
		}

		if (memops::strcmp(sched_alg_name, "sfs") == 0) {
			sched_alg_ = new alg::simple_fair_scheduler();
		} else if (memops::strcmp(sched_alg_name, "rr") == 0) {
			sched_alg_ = new alg::round_robin();
		} else {
			panic("Unsupported scheduling algorithm '%s'", sched_alg_name);
		}

		dprintf("core: using scheduling algorithm: %s\n", sched_alg_->name());
	}

	int id() const { return id_; }

	virtual void init() = 0;

	virtual bool remote_run() = 0;
	__noreturn void run();

	virtual timer &local_timer() = 0;

	void add_to_runqueue(tcb &tcb) { sched_alg_->add_to_runqueue(tcb); }
	void remove_from_runqueue(tcb &tcb) { sched_alg_->remove_from_runqueue(tcb); }

	void schedule();

	virtual void set_current_tcb(const tcb *tcb) = 0;
	virtual tcb *get_current_tcb() = 0;

	core_status status() const { return status_; }

	irq_manager &irqs() { return irqs_; }
	const irq_manager &irqs() const { return irqs_; }

	void update_accounting();

private:
	int id_;
	core_status status_;
	irq_manager irqs_;

	tcb idle_thread_;
	alg::scheduling_algorithm *sched_alg_;
};
} // namespace stacsos::kernel::arch
