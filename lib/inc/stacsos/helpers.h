/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
template <class T> struct remove_reference {
	typedef T type;
};

template <class T> struct remove_reference<T &> {
	typedef T type;
};

template <class T> struct remove_reference<T &&> {
	typedef T type;
};

template <class T> using remove_reference_type = remove_reference<T>::type;

template <class T> constexpr remove_reference_type<T> &&move(T &&t) noexcept { return static_cast<remove_reference_type<T> &&>(t); }

template <class T> constexpr void swap(T &a, T &b) noexcept
{
	T t = move(a);
	a = move(b);
	b = t;
}

template <class T> constexpr T &&forward(remove_reference_type<T> &t) noexcept { return static_cast<T &&>(t); }

template <class T> constexpr T &&forward(remove_reference_type<T> &&t) noexcept { return static_cast<T &&>(t); }

} // namespace stacsos
