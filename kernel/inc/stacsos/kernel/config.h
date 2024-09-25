#pragma once

#include <stacsos/memops.h>

namespace stacsos::kernel {
struct config_option {
	const char *key;
	const char *value;
};

class config {
	DEFINE_SINGLETON(config)

private:
	config()
		: nr_options_(0)
	{
	}

public:
	void init(const char *cmdline);

	const char *get_option(const char *name) const
	{
		for (unsigned int i = 0; i < nr_options_; i++) {
			if (memops::strcmp(options_[i].key, name) == 0) {
				return options_[i].value;
			}
		}

		return nullptr;
	}

	const char *get_option_or_default(const char *name, const char *dfl) const
	{
		for (unsigned int i = 0; i < nr_options_; i++) {
			if (memops::strcmp(options_[i].key, name) == 0) {
				return options_[i].value;
			}
		}

		return dfl;
	}

private:
	char command_line_[256];
	config_option options_[32];
	unsigned int nr_options_;

	void add_option(const char *key, const char *value);
};
} // namespace stacsos::kernel
