/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos::kernel::arch::x86 {
static void __cpuid(u32 &eax, u32 &ebx, u32 &ecx, u32 &edx) { asm volatile("cpuid" : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx)); }

enum class cpuid_features {
#define feature(__name, __leaf, __reg, __bit) __name,
#define feature2(__name, __leaf, __add, __reg, __bit) __name,
#include <stacsos/kernel/arch/x86/cpuid-features.h>
#undef feature
#undef feature2
	__cpuid_features_end
};

enum class cpuid_feature_reg { eax, ebx, ecx, edx };

struct cpuid_mapping {
	int fn, ext;
	cpuid_feature_reg rg;
	int bit;
	cpuid_features feat;
	const char *name;
	bool value;
};

class cpuid {
public:
	cpuid();

	void initialise();
	void dump() const;

	bool get_feature(cpuid_features feature) const { return mapping_[static_cast<int>(feature)].value; }

	void mfr_id(u32 &max_func, char *buffer) const
	{
		u32 ebx = 0, ecx = 0, edx = 0;

		max_func = 0;
		__cpuid(max_func, ebx, ecx, edx);

		buffer[0] = ((const char *)&ebx)[0];
		buffer[1] = ((const char *)&ebx)[1];
		buffer[2] = ((const char *)&ebx)[2];
		buffer[3] = ((const char *)&ebx)[3];
		buffer[4] = ((const char *)&edx)[0];
		buffer[5] = ((const char *)&edx)[1];
		buffer[6] = ((const char *)&edx)[2];
		buffer[7] = ((const char *)&edx)[3];
		buffer[8] = ((const char *)&ecx)[0];
		buffer[9] = ((const char *)&ecx)[1];
		buffer[10] = ((const char *)&ecx)[2];
		buffer[11] = ((const char *)&ecx)[3];
	}

private:
	cpuid_mapping mapping_[static_cast<int>(cpuid_features::__cpuid_features_end) + 1];
};

} // namespace stacsos::kernel::arch::x86
