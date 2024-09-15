/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch {
class core;

struct core_enumerator {
	core_enumerator(core **core_array, size_t count)
		: core_array_(core_array)
		, count_(count)
	{
	}

	struct core_iterator {
		using value_type = core *;

		core_iterator(core_enumerator &en, size_t offset)
			: en_(en)
			, off_(offset)
		{
			progress();
		}

		core_iterator(const core_iterator &o)
			: en_(o.en_)
			, off_(o.off_)
		{
		}

		core_iterator(core_iterator &&o)
			: en_(o.en_)
			, off_(o.off_)
		{
		}

		value_type operator*() const { return en_.core_array_[off_]; }
		value_type operator->() { return en_.core_array_[off_]; }

		core_iterator &operator++()
		{
			progress();
			return *this;
		}

		friend bool operator==(const core_iterator &a, const core_iterator &b) { return a.off_ == b.off_; }

		friend bool operator!=(const core_iterator &a, const core_iterator &b) { return a.off_ != b.off_; }

	private:
		core_enumerator &en_;
		size_t off_;

		void progress()
		{
			while (off_ < en_.count_ && en_.core_array_[off_] == nullptr) {
				off_++;
			}
		}
	};

	core_iterator begin() { return core_iterator(*this, 0); }
	core_iterator end() { return core_iterator(*this, count_); }

private:
	core **core_array_;
	size_t count_;
};

class core_manager {
public:
	DEFINE_SINGLETON(core_manager)

	static const int max_cores = 8;

	core_manager()
		: nr_cores_(0)
	{
		for (int i = 0; i < max_cores; i++) {
			cores_[i] = nullptr;
		}
	}

	void init();

	core &get_core(int id) const
	{
		if (id >= max_cores || cores_[id] == nullptr) {
			panic("invalid core id");
		}

		return *cores_[id];
	}

	core &get_boot_core() const { return get_core(0); }

	void register_core(core &c);

	__noreturn void go();

	core_enumerator cores() { return core_enumerator(cores_, max_cores); }

private:
	core *cores_[max_cores];
	int nr_cores_;
};
} // namespace stacsos::kernel::arch
