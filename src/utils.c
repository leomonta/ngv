#include "utils.h"

bool at_bit(const bitfield bf, const unsigned char i) {
	if (i >= (sizeof(bf) * 8)) {
		// OOB false
		return false;
	}

	// bitfield mask = 1 << i;
	// bool res = bf & mask != 0;
	return (bf & (1 << i)) != 0;
}

bool at_bit_long(const longBitfield lbf, const unsigned char i) {

	if (i >= sizeof(lbf) * 8) {
		// OOB false
		return false;
	}

	// bitfield mask = 1 << i;
	// bool res = bf & mask != 0;
	return (lbf & (1 << i)) != 0;
}

uint32_t popcnt(bitfield bf) {
	uint32_t cnt = 0;

		// feeling shfancy

		// Doin' this cuz i can
		// only on gcc / clang (for the fany asm) + x86 (for the actual popcnt instruction)
#if  defined (__GNUC__) && defined(__x86_64__)
		__asm__ ("popcnt %0, %1"
				: "=r"(cnt)
				: "r"(bf));
#else
		for (unsigned char i = 0; i < (sizeof(bitfield)); ++i) {
		if (at_bit(bf, i)) {
			++cnt;
		}
	}
#endif
	return cnt;
}
