/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
struct nullopt_t {
	constexpr explicit nullopt_t(int) { }
};

constexpr nullopt_t nullopt { 42 };

template <class T> class optional {
public:
	constexpr optional() noexcept
		: none_(0)
		, has_value_(false)
	{
	}

	constexpr optional(nullopt_t) noexcept
		: none_(0)
		, has_value_(false)
	{
	}

	optional(const optional &o)
		: none_(0)
		, has_value_(false)
	{
		if (o.has_value_) {
			has_value_ = true;
			value_ = o.value_;
		}
	}

	optional(optional &&o)
		: none_(0)
		, has_value_(false)
	{
		if (o.has_value_) {
			has_value_ = true;
			value_ = move(o.value_);
		}
	}

	constexpr optional(const T &value)
		: value_(value)
		, has_value_(true)
	{
	}

	constexpr optional(T &&value)
		: value_(move(value))
		, has_value_(true)
	{
	}

	~optional()
	{
		if (has_value_) {
			value_.~T();
		}
	}

	optional &operator=(nullopt_t) noexcept
	{
		has_value_ = false;
		return *this;
	}

	template <class U> optional &operator=(U &&v)
	{
		has_value_ = true;
		value_ = move(v);
		return *this;
	}

	template <class U> optional &operator=(const U &v)
	{
		has_value_ = true;
		value_ = v;
		return *this;
	}

	explicit operator bool() const noexcept { return has_value_; }

	T &value() & { return has_value_ ? value_ : panic("attempt to access value from disengaged optional"); }
	const T &value() const & { return has_value_ ? value_ : panic("attempt to access value from disengaged optional"); }
	T &&value() && { return has_value_ ? move(value_) : panic("attempt to access value from disengaged optional"); }
	const T &&value() const && { return has_value_ ? move(value_) : panic("attempt to access value from disengaged optional"); }

	bool has_value() const { return has_value_; }

private:
	union {
		char none_;
		T value_;
	};

	bool has_value_;
};
} // namespace stacsos
