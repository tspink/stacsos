/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long int;
using s8 = signed char;
using s16 = signed short;
using s32 = signed int;
using s64 = signed long int;
using size_t = unsigned long;
using uintptr_t = unsigned long;
using intptr_t = signed long;
using off_t = signed long;
using ptrdiff_t = signed long;
using intmax_t = s64;
using uintmax_t = u64;
using max_align_t = u64;

static const u32 U32_MAX = 0xffffffffu;
static const u16 U16_MAX = 0xffffu;

static const unsigned long PAGE_BITS = 12;
static const unsigned long PAGE_SIZE = 1 << PAGE_BITS;
static const unsigned long PAGE_MASK = ~(PAGE_SIZE - 1ull);

#define PAGE_ALIGN_DOWN(__addr) ((__addr) & PAGE_MASK)
#define PAGE_ALIGN_UP(__addr) (((__addr) + (PAGE_SIZE - 1ull)) & PAGE_MASK)

/*
 * VA List Helpers
 */
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

/*
 * Attributes
 */
#define __weak __attribute__((weak))
#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#define __ctor __attribute__((constructor))
#define __ctor_priority(__n) __attribute__((constructor(__n)))
#define __pure __attribute__((pure))
#define __aligned(__n) __attribute__((aligned(__n)))
#define __section(__n) __attribute__((section(__n)))
#define __init_priority(__n) __attribute__((init_priority(__n)))
#define __noderef __attribute__((noderef))
#define __nocast __attribute__((nocast))
#define __visibility(__n) __attribute__((visibility(__n)))
#define __unreachable() __builtin_unreachable()
#define __constexpr constexpr

/*
 * Code flow analysis
 */
#define unlikely(__expr) __expr
#define likely(__expr) __expr

/* Utilities */
#define ARRAY_SIZE(__arr) (sizeof(__arr) / sizeof(__arr[0]))

#define KB(n) ((n) * 1024ull)
#define MB(n) (KB(n) * 1024ull)
#define GB(n) (MB(n) * 1024ull)

/* Class definition helpers */
#define APPLY_ENUM_FLAG_OP(__enum_typename, __op)                                                                                                              \
	static_cast<__enum_typename>(static_cast<__underlying_type(__enum_typename)>(a) __op static_cast<__underlying_type(__enum_typename)>(b))

#define APPLY_ENUM_FLAG_REF_OP(__enum_typename, __op)                                                                                                          \
	reinterpret_cast<__enum_typename &>(reinterpret_cast<__underlying_type(__enum_typename) &>(a) __op static_cast<__underlying_type(__enum_typename)>(b))

#define DEFINE_ENUM_FLAG_OPERATIONS(__enum_typename)                                                                                                           \
	inline __enum_typename operator|(__enum_typename a, __enum_typename b) { return APPLY_ENUM_FLAG_OP(__enum_typename, |); }                                  \
	inline __enum_typename &operator|=(__enum_typename &a, __enum_typename b) { return APPLY_ENUM_FLAG_REF_OP(__enum_typename, |=); }                          \
	inline __enum_typename operator&(__enum_typename a, __enum_typename b) { return APPLY_ENUM_FLAG_OP(__enum_typename, &); }                                  \
	inline __enum_typename &operator&=(__enum_typename &a, __enum_typename b) { return APPLY_ENUM_FLAG_REF_OP(__enum_typename, &=); }                          \
	inline __enum_typename operator~(__enum_typename a) { return static_cast<__enum_typename>(~static_cast<__underlying_type(__enum_typename)>(a)); }

#define DELETE_DEFAULT_COPY_AND_MOVE(__class_typename)                                                                                                         \
	__class_typename(const __class_typename &) = delete;                                                                                                       \
	__class_typename(__class_typename &&) = delete;                                                                                                            \
	__class_typename &operator=(__class_typename) = delete;

#define DEFINE_SINGLETON(__class_typename)                                                                                                                     \
private:                                                                                                                                                       \
	DELETE_DEFAULT_COPY_AND_MOVE(__class_typename)                                                                                                             \
public:                                                                                                                                                        \
	static auto &get()                                                                                                                                         \
	{                                                                                                                                                          \
		static __class_typename i;                                                                                                                             \
		return i;                                                                                                                                              \
	}

/*
 * Mathematics
 */
template <typename I> static inline I log2_ceil(I v)
{
	I r;
	asm("\tbsr %1, %0\n" : "=r"(r) : "r"(v));

	return r + (v & (v - 1) ? 1 : 0);
}

template <typename I> static inline I log2(I v)
{
	I r;
	asm("\tbsr %1, %0\n" : "=r"(r) : "r"(v));

	return r;
}

template <typename I> static inline I pow2(I v) { return (I)1 << v; }

template <typename I> static inline I max(I a, I b) { return a > b ? a : b; }

template <typename I> static inline I min(I a, I b) { return a < b ? a : b; }

template <typename I> static inline I byte_swap(I value)
{
	asm("bswap %0" : "=r"(value) : "0"(value));
	return value;
}

template <> inline u16 byte_swap(u16 value)
{
	asm("ror $8, %0" : "=r"(value) : "0"(value));
	return value;
}

static inline void __relax() { asm volatile("pause"); }

#ifndef __i386__
/* Placement new operator */
inline void *operator new(size_t size, void *p) { return p; }
#endif

/* Global functions */
static inline __noreturn void abort()
{
	asm volatile("cli");
	for (;;) {
		asm volatile("hlt");
	}
	__unreachable();
}

static inline __noreturn void hang_loop()
{
	for (;;) {
		__relax();
	}
	__unreachable();
}

extern __noreturn void panic(const char *fmt, ...);
extern __noreturn void panic_with_ctx(const void *mctx, const char *fmt, ...);

/* Assertions */
static inline void __assert(bool cond, const char *str)
{
	if (!cond) {
		panic("ASSERTION FAILED: %s", str);
		__unreachable();
	}
}

#define assert(cond_) __assert((cond_), #cond_)
