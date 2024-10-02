/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/printf.h>

namespace stacsos::kernel {
enum class log_level { trace, debug, info, warning, error, panic };

class logger {
public:
	static logger root_logger;

	logger(logger &parent, const char *log_source)
		: parent_(&parent)
		, log_source_(log_source)
		, enabled_(true)
		, min_level_(log_level::trace)
	{
	}

	void log(log_level level, const char *message) { internal_log(log_source_, level, message); }

	void logf(log_level level, const char *fmt, ...)
	{
		char format_buffer[256];
		va_list args;
		va_start(args, fmt);
		vsnprintf(format_buffer, sizeof(format_buffer) - 1, fmt, args);
		va_end(args);

		log(level, format_buffer);
	}

private:
	explicit logger()
		: parent_(nullptr)
		, log_source_(nullptr)
		, enabled_(true)
		, min_level_(log_level::trace)
	{
	}

	void internal_log(const char *log_source, log_level level, const char *message)
	{
		// Apply filter.
		if (!enabled_ || level < min_level_) {
			return;
		}

		// Propagate, or emit.
		if (parent_) {
			parent_->internal_log(log_source, level, message);
		} else {
			emit(log_source, level, message);
		}
	}

	void emit(const char *log_source, log_level level, const char *message);

	logger *parent_;
	const char *log_source_;
	bool enabled_;
	log_level min_level_;
};
} // namespace stacsos::kernel
