/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
class software_based_memops {
public:
	static void bzero(void *ptr, size_t size) { memset(ptr, 0, size); }

	static void pzero(void *ptr, size_t count) { bzero(ptr, count << PAGE_BITS); }

	static void *memset(void *dest, int c, size_t size)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
		for (size_t i = 0; i < size; i++) {
			((u8 *)dest)[i] = c;
		}

#pragma GCC diagnostic pop

		return dest;
	}

	static void *memcpy(void *dest, const void *src, size_t size)
	{
		for (size_t i = 0; i < size; i++) {
			((u8 *)dest)[i] = ((const u8 *)src)[i];
		}

		return dest;
	}

	static int memcmp(const void *a, const void *b, size_t size)
	{
		for (size_t i = 0; i < size; i++) {
			if (((const u8 *)a)[i] != ((const u8 *)b)[i]) {
				return -1;
			}
		}

		return 0;
	}

	static size_t strlen(const char *str)
	{
		size_t count = 0;
		while (*str++) {
			count++;
		}

		return count;
	}

	static void *strncpy(char *dest, const char *src, size_t length) { return memcpy(dest, src, strlen(src)); }
};

extern "C" void __x86_bzero(void *, size_t);
extern "C" void __x86_pzero(void *, size_t);
extern "C" void *__x86_memset(void *, int, size_t);
extern "C" void *__x86_memcpy(void *, const void *, size_t);
extern "C" int __x86_memcmp(const void *, const void *, size_t);
extern "C" size_t __x86_strlen(const char *);
extern "C" void *__x86_strncpy(char *, const char *, size_t);
extern "C" int __x86_strcmp(const char *, const char *);

class native_memops {

public:
	static void bzero(void *ptr, size_t size) { __x86_bzero(ptr, size); }

	static void pzero(void *ptr, size_t count) { return __x86_pzero(ptr, count); }

	static void *memset(void *dest, int c, size_t size) { return __x86_memset(dest, c, size); }

	static void *memcpy(void *dest, const void *src, size_t size) { return __x86_memcpy(dest, src, size); }

	static int memcmp(const void *a, const void *b, size_t size) { return __x86_memcmp(a, b, size); }

	static size_t strlen(const char *str) { return __x86_strlen(str); }

	static void *strncpy(char *dest, const char *src, size_t length) { return __x86_strncpy(dest, src, length); }

	static int strcmp(const char *str1, const char *str2) { return __x86_strcmp(str1, str2); }
};

template <class Impl> class memops_carrier {
public:
	static void bzero(void *ptr, size_t size) { Impl::bzero(ptr, size); }
	static void pzero(void *ptr, size_t count) { Impl::pzero(ptr, count); }

	static void *memcpy(void *dest, const void *src, size_t size) { return Impl::memcpy(dest, src, size); }
	static void *memset(void *dest, int c, size_t size) { return Impl::memset(dest, c, size); }
	static int memcmp(const void *ptr1, const void *ptr2, size_t num) { return Impl::memcmp(ptr1, ptr2, num); }

	static int strlen(const char *str) { return Impl::strlen(str); }
	static void *strncpy(char *dest, const char *src, size_t length) { return Impl::strncpy(dest, src, length); }
	static int strcmp(const char *str1, const char *str2) { return Impl::strcmp(str1, str2); }
};

using memops = memops_carrier<native_memops>;
} // namespace stacsos
