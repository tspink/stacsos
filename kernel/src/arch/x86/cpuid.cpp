#include <stacsos/kernel/arch/x86/cpuid.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos::kernel::arch::x86;

cpuid::cpuid()
{
#define feature(__name, __leaf, __reg, __bit)                                                                                                                  \
	mapping_[(int)cpuid_features::__name] = { __leaf, 0, cpuid_feature_reg::__reg, __bit, cpuid_features::__name, #__name, false };
#define feature2(__name, __leaf, __add, __reg, __bit)                                                                                                          \
	mapping_[(int)cpuid_features::__name] = { __leaf, __add, cpuid_feature_reg::__reg, __bit, cpuid_features::__name, #__name, false };
#include <stacsos/kernel/arch/x86/cpuid-features.h>
#undef feature
#undef feature2
}

void cpuid::initialise()
{
	for (unsigned int i = 0; i < ARRAY_SIZE(mapping_); i++) {
		u32 eax, ebx, ecx, edx;
		eax = mapping_[i].fn;
		ebx = 0;
		ecx = mapping_[i].ext;
		edx = 0;

		__cpuid(eax, ebx, ecx, edx);

		switch (mapping_[i].rg) {
		case cpuid_feature_reg::eax:
			mapping_[i].value = !!(eax & (1u << mapping_[i].bit));
			break;
		case cpuid_feature_reg::ebx:
			mapping_[i].value = !!(ebx & (1u << mapping_[i].bit));
			break;
		case cpuid_feature_reg::ecx:
			mapping_[i].value = !!(ecx & (1u << mapping_[i].bit));
			break;
		case cpuid_feature_reg::edx:
			mapping_[i].value = !!(edx & (1u << mapping_[i].bit));
			break;
		}
	}
}

void cpuid::dump() const
{
	dprintf("cpu features: ");
	for (unsigned int i = 0; i < ARRAY_SIZE(mapping_); i++) {
		if (mapping_[i].value) {
			dprintf("%s ", mapping_[i].name);
		}
	}

	dprintf("\n");
}
