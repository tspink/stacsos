/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/memops.h>

namespace stacsos {
template <size_t nr_bits, typename bit_word = unsigned long> class bitset {
	static_assert(nr_bits != 0);
	static const size_t bits_per_word = sizeof(bit_word) * 8;
	static const size_t nr_words = (nr_bits + (bits_per_word - 1)) / bits_per_word;

public:
	struct bitref {
		bit_word &word;
		u64 offset;

		bitref &operator=(bool v)
		{
			word = (word & (~((bit_word)1 << offset))) | (((bit_word) !!v) << offset);
			return *this;
		}

		operator bool() const { return !!(word & ((bit_word)1 << offset)); }
	};

	bitref operator[](u64 index) { return bitref { words_[index / bits_per_word], index % bits_per_word }; }

	u64 find_first_zero()
	{
		for (u64 i = 0; i < nr_words; i++) {
			bit_word tmp = ~words_[i];

			int first_set = __builtin_ffsll(tmp);
			if (first_set != 0) {
				return (u64)(first_set - 1) + (i * sizeof(bit_word) * 8);
			}
		}

		return ~0ull;
	}

	bitset() { memops::bzero(words_, sizeof(words_)); }

private:
	bit_word words_[nr_words];
};
} // namespace stacsos
