/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
class object;

class console {
public:
	static console &get()
	{
		static console c;
		return c;
	}

	void init();

	void write(const char *msg);
	void writef(const char *msg, ...);
	char read_char();

private:
	console()
		: console_object_(nullptr)
	{
	}

	object *console_object_;
};
} // namespace stacsos
